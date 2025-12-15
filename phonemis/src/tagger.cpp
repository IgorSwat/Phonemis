#include <phonemis/tagger/tagger.h>
#include <phonemis/utilities/io_utils.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <stdexcept>

namespace phonemis::tagger {

Tagger::Tagger(const std::string& hmm_data_path) {
	// Load the input JSON file
	nlohmann::json json_obj = utilities::io_utils::load_json(hmm_data_path);

	// Validate required top-level fields
	if (!json_obj.contains("start_prob") || !json_obj.contains("emission") || !json_obj.contains("transition")) {
		throw std::invalid_argument("JSON missing required fields: start_prob, emission, transition");
	}
	if (!json_obj["start_prob"].is_object() || !json_obj["emission"].is_object() || !json_obj["transition"].is_object()) {
		throw std::invalid_argument("JSON fields must be objects: start_prob, emission, transition");
	}

	// Load start probabilities
  // We can simultaneously load all the possible tags here, since
  // all the tags must appear in start_prob field of the JSON file.
	for (auto& item : json_obj["start_prob"].items()) {
		const Tag tag = item.key();
    tags_.insert(tag);
		start_probs_[tag] = item.value().get<double>();
	}

	// Load emission probabilities
	for (auto& tag_item : json_obj["emission"].items()) {
		const Tag tag = tag_item.key();
		const auto& inner = tag_item.value();
		if (!inner.is_object()) continue;
		for (auto& w : inner.items()) {
			emission_probs_[tag][w.key()] = w.value().get<double>();
		}
	}

	// Load transition probabilities
	for (auto& tag_item : json_obj["transition"].items()) {
		const Tag tag = tag_item.key();
		const auto& inner = tag_item.value();
		if (!inner.is_object()) continue;
		for (auto& t : inner.items()) {
			transition_probs_[tag][t.key()] = t.value().get<double>();
		}
	}
}

void Tagger::tag(std::vector<tokenizer::Token> &sentence) const {
  // A workaround for zero probability
  constexpr double EPSILON = 1e-6;

	if (sentence.empty()) {
		return;
	}

	// Viterbi tables
  // back_pointer table allows to reconstruct the optimal path in the state (tag) graph.
	std::vector<std::unordered_map<Tag, double>> 
    v(sentence.size()); // v[t][state] -> probability
	std::vector<std::unordered_map<Tag, Tag>> 
    back_pointer(sentence.size());  // back_pointer[t][state] -> previous_state

	// Initialization
  // Calculates probabilities for the first word in the sentence.
	for (const auto& tag : tags_) {
		double start_p = start_probs_.at(tag);
    double emit_p = emission_probs_.at(tag).contains(sentence[0].text) 
                    ? emission_probs_.at(tag).at(sentence[0].text) : EPSILON;
		v[0][tag] = start_p * emit_p;

		// To make the algorithm less case-sensitive, probe the initial value for lower-case word
		if (std::isalpha(sentence[0].text[0])) {
			std::string lowerized = sentence[0].text;
			lowerized[0] = std::tolower(lowerized[0]);
			emit_p = emission_probs_.at(tag).contains(lowerized) 
                ? emission_probs_.at(tag).at(lowerized) : EPSILON;
			v[0][tag] = std::max(v[0][tag], start_p * emit_p);
		}
	}

	// Recursion
  // Processes through the rest of the sentence.
	for (size_t t = 1; t < sentence.size(); ++t) {
    const auto& word = sentence[t].text;

		for (const auto& curr_tag : tags_) {
      // Helper variables to track the best branch
			double max_prob = -1.0;
			Tag best_prev;

      double emit_p = emission_probs_.at(curr_tag).contains(word) 
                    ? emission_probs_.at(curr_tag).at(word) : EPSILON;

			for (const auto& prev_tag : tags_) {
        double trans_p = transition_probs_.at(prev_tag).contains(curr_tag)
                          ? transition_probs_.at(prev_tag).at(curr_tag) : EPSILON;
				double prob = v[t - 1][prev_tag] * trans_p * emit_p;

				if (prob > max_prob) {
					max_prob = prob;
					best_prev = prev_tag;
				}
			}

			v[t][curr_tag] = max_prob;
			back_pointer[t][curr_tag] = best_prev;
		}
	}

	// Termination
  // Selects the most probable final tag.
  // The other tags are selected by backtracking through the saved path.
	size_t last_idx = sentence.size() - 1;
	auto best_it = std::max_element(tags_.begin(), tags_.end(), [&](const auto& a, const auto& b) {
			return v[last_idx][a] < v[last_idx][b];
	});

	// Backtracking path
	Tag current_tag = *best_it;
	sentence[last_idx].tag = current_tag;

	for (size_t t = last_idx; t > 0; --t) {
		current_tag = back_pointer[t][current_tag];
		sentence[t - 1].tag = current_tag;
	}
}

} // namespace phonemis::tagger