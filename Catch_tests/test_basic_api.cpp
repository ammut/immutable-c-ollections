//
// Created by sam on 20.03.2020.
//

#define CHAMP_KEY_T char*
#define CHAMP_VALUE_T int*

extern "C" {
#include "champ.h"
#include "champ_fns.h"

struct node {
	uint8_t element_arity;
	uint8_t branch_arity;
	uint16_t ref_count;
	uint32_t element_map;
	uint32_t branch_map;
	struct {CHAMP_KEY_T a; CHAMP_VALUE_T b;} content[];
};

struct collision_node {
	uint8_t element_arity;
	uint8_t branch_arity;
	uint16_t ref_count;
	struct {CHAMP_KEY_T a; CHAMP_VALUE_T b;} content[];
};
}

#include <fstream>
#include <string>
#include <map>
#include <iostream>
#include "catch.hpp"


int hash_calls = 0;
uint32_t hash_mock(const char *str) {
	++hash_calls;
	return champ_hash_str(str);
};

int equals_calls = 0;
int equals_mock(const char *l, const char *r) {
	++equals_calls;
	return champ_equals_str(l, r);
};

SCENARIO("The default champ implementation") {
	GIVEN("a champ<string, ?>") {

		WHEN("A new empty map is created") {
			auto empty_map = champ_new(hash_mock, equals_mock);

			THEN("It should have a length of zero") {
				REQUIRE(champ_length(empty_map) == 0);
			}

			THEN("It's root node should be empty") {
				REQUIRE(empty_map->root->element_arity == 0);
				REQUIRE(empty_map->root->branch_arity == 0);
				REQUIRE(empty_map->root->branch_map == 0);
				REQUIRE(empty_map->root->element_map == 0);
			}

			champ_destroy(&empty_map);
		}

		WHEN("A new map with two initial values is created") {
			const char *keys[] = {"foo", "bar"};

			int i1 = 21, i2 = 42;
			int *values[] = {&i1, &i2};

			auto map2 = champ_of(hash_mock, equals_mock, (char **)keys, (int **)values, 2);

			THEN("It should have a length of two") {
				REQUIRE(champ_length(map2) == 2);
			}

			THEN("The provided keys should map to the provided values") {
				int found;

				REQUIRE(*champ_get(map2, "foo", &found) == 21);
				REQUIRE(found == 1);

				REQUIRE(*champ_get(map2, "bar", &found) == 42);
				REQUIRE(found == 1);
			}

			champ_destroy(&map2);
		}
	}

	GIVEN("An empty map") {
		auto map0 = champ_new(hash_mock, equals_mock);
		auto empty_node = map0->root;

		WHEN("Initializing") {
			THEN("It should be empty") {
				REQUIRE(champ_length(map0) == 0);
				REQUIRE(map0->root->branch_map == 0);
				REQUIRE(map0->root->element_map == 0);
			}
		}

		WHEN("A new key is added") {
			const int foo = 321;
			hash_calls = 0;
			equals_calls = 0;
			int replaced = 0;
			auto map1 = champ_set(map0, "foo", &foo, &replaced);

			THEN("An updated map should be returned") {
				int found;
				REQUIRE(champ_length(map1) == 1);
				REQUIRE(*champ_get(map1, "foo", &found) == 321);
				REQUIRE(found == 1);
			}

			THEN("The original map should still be empty") {
				REQUIRE(champ_length(map0) == 0);
				REQUIRE(map0->root == empty_node);
				REQUIRE(map0->root->branch_map == 0);
				REQUIRE(map0->root->element_map == 0);
			}

			THEN("The hash function should be called exactly once") {
				REQUIRE(hash_calls == 1);
			}

			THEN("The equals function should not be called") {
				REQUIRE(equals_calls == 0);
			}

			THEN("The replaced flag should be false") {
				REQUIRE(replaced == 0);

				WHEN("The same key is added again") {
					int foo2 = 432;

					auto map2 = champ_set(map1, "foo", &foo2, &replaced);

					THEN("The replaced flag should be true") {
						REQUIRE(replaced == 1);
					}

					THEN("The value should be updated") {
						REQUIRE(*champ_get(map2, "foo", nullptr) == 432);
					}

					champ_destroy(&map2);
				}
			}

			champ_destroy(&map1);
		}

		WHEN("Multiple different keys are added") {
			const char *keys[] = {"a", "b", "c", "d", "e", "f", "g"};
			const int values[] = {321, 432, 543, 654, 765, 876, 987};

			auto map1 = champ_set(map0, keys[0], &values[0], nullptr);
			auto map2 = champ_set(map1, keys[1], &values[1], nullptr);
			auto map3 = champ_set(map2, keys[2], &values[2], nullptr);
			auto map4 = champ_set(map3, keys[3], &values[3], nullptr);
			auto map5 = champ_set(map4, keys[4], &values[4], nullptr);
			auto map6 = champ_set(map5, keys[5], &values[5], nullptr);
			auto map7 = champ_set(map6, keys[6], &values[6], nullptr);

			THEN("A new map should be created for every update") {
				REQUIRE(map0 != map1);
				REQUIRE(map0 != map2);
				REQUIRE(map0 != map3);
				REQUIRE(map0 != map4);
				REQUIRE(map0 != map5);
				REQUIRE(map0 != map6);
				REQUIRE(map0 != map7);

				REQUIRE(map1 != map2);
				REQUIRE(map1 != map3);
				REQUIRE(map1 != map4);
				REQUIRE(map1 != map5);
				REQUIRE(map1 != map6);
				REQUIRE(map1 != map7);

				REQUIRE(map2 != map3);
				REQUIRE(map2 != map4);
				REQUIRE(map2 != map5);
				REQUIRE(map2 != map6);
				REQUIRE(map2 != map7);

				REQUIRE(map3 != map4);
				REQUIRE(map3 != map5);
				REQUIRE(map3 != map6);
				REQUIRE(map3 != map7);

				REQUIRE(map4 != map5);
				REQUIRE(map4 != map6);
				REQUIRE(map4 != map7);

				REQUIRE(map5 != map6);
				REQUIRE(map5 != map7);

				REQUIRE(map6 != map7);
			}

			THEN("The original map should be empty") {
				REQUIRE(champ_length(map0) == 0);
				REQUIRE(champ_get(map0, keys[0], nullptr) == nullptr);
				REQUIRE(champ_get(map0, keys[1], nullptr) == nullptr);
				REQUIRE(champ_get(map0, keys[2], nullptr) == nullptr);
				REQUIRE(champ_get(map0, keys[3], nullptr) == nullptr);
				REQUIRE(champ_get(map0, keys[4], nullptr) == nullptr);
				REQUIRE(champ_get(map0, keys[5], nullptr) == nullptr);
				REQUIRE(champ_get(map0, keys[6], nullptr) == nullptr);
			}

			THEN("map1 should have only 1 value") {
				REQUIRE(champ_length(map1) == 1);
				REQUIRE(*champ_get(map1, keys[0], nullptr) == values[0]);
				REQUIRE(champ_get(map1, keys[1], nullptr) == nullptr);
				REQUIRE(champ_get(map1, keys[2], nullptr) == nullptr);
				REQUIRE(champ_get(map1, keys[3], nullptr) == nullptr);
				REQUIRE(champ_get(map1, keys[4], nullptr) == nullptr);
				REQUIRE(champ_get(map1, keys[5], nullptr) == nullptr);
				REQUIRE(champ_get(map1, keys[6], nullptr) == nullptr);
			}

			THEN("map2 should have only 2 values") {
				REQUIRE(champ_length(map2) == 2);
				REQUIRE(*champ_get(map2, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(map2, keys[1], nullptr) == values[1]);
				REQUIRE(champ_get(map2, keys[2], nullptr) == nullptr);
				REQUIRE(champ_get(map2, keys[3], nullptr) == nullptr);
				REQUIRE(champ_get(map2, keys[4], nullptr) == nullptr);
				REQUIRE(champ_get(map2, keys[5], nullptr) == nullptr);
				REQUIRE(champ_get(map2, keys[6], nullptr) == nullptr);
			}

			THEN("map3 should have only 3 values") {
				REQUIRE(champ_length(map3) == 3);
				REQUIRE(*champ_get(map3, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(map3, keys[1], nullptr) == values[1]);
				REQUIRE(*champ_get(map3, keys[2], nullptr) == values[2]);
				REQUIRE(champ_get(map3, keys[3], nullptr) == nullptr);
				REQUIRE(champ_get(map3, keys[4], nullptr) == nullptr);
				REQUIRE(champ_get(map3, keys[5], nullptr) == nullptr);
				REQUIRE(champ_get(map3, keys[6], nullptr) == nullptr);
			}

			THEN("map4 should have only 4 values") {
				REQUIRE(champ_length(map4) == 4);
				REQUIRE(*champ_get(map4, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(map4, keys[1], nullptr) == values[1]);
				REQUIRE(*champ_get(map4, keys[2], nullptr) == values[2]);
				REQUIRE(*champ_get(map4, keys[3], nullptr) == values[3]);
				REQUIRE(champ_get(map4, keys[4], nullptr) == nullptr);
				REQUIRE(champ_get(map4, keys[5], nullptr) == nullptr);
				REQUIRE(champ_get(map4, keys[6], nullptr) == nullptr);
			}

			THEN("map5 should have only 5 values") {
				REQUIRE(champ_length(map5) == 5);
				REQUIRE(*champ_get(map5, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(map5, keys[1], nullptr) == values[1]);
				REQUIRE(*champ_get(map5, keys[2], nullptr) == values[2]);
				REQUIRE(*champ_get(map5, keys[3], nullptr) == values[3]);
				REQUIRE(*champ_get(map5, keys[4], nullptr) == values[4]);
				REQUIRE(champ_get(map5, keys[5], nullptr) == nullptr);
				REQUIRE(champ_get(map5, keys[6], nullptr) == nullptr);
			}

			THEN("map6 should have only 6 values") {
				REQUIRE(champ_length(map6) == 6);
				REQUIRE(*champ_get(map6, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(map6, keys[1], nullptr) == values[1]);
				REQUIRE(*champ_get(map6, keys[2], nullptr) == values[2]);
				REQUIRE(*champ_get(map6, keys[3], nullptr) == values[3]);
				REQUIRE(*champ_get(map6, keys[4], nullptr) == values[4]);
				REQUIRE(*champ_get(map6, keys[5], nullptr) == values[5]);
				REQUIRE(champ_get(map6, keys[6], nullptr) == nullptr);
			}

			THEN("map7 should have all 7 values") {
				REQUIRE(champ_length(map7) == 7);
				REQUIRE(*champ_get(map7, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(map7, keys[1], nullptr) == values[1]);
				REQUIRE(*champ_get(map7, keys[2], nullptr) == values[2]);
				REQUIRE(*champ_get(map7, keys[3], nullptr) == values[3]);
				REQUIRE(*champ_get(map7, keys[4], nullptr) == values[4]);
				REQUIRE(*champ_get(map7, keys[5], nullptr) == values[5]);
				REQUIRE(*champ_get(map7, keys[6], nullptr) == values[6]);
			}

			champ_destroy(&map1);
			champ_destroy(&map2);
			champ_destroy(&map3);
			champ_destroy(&map4);
			champ_destroy(&map5);
			champ_destroy(&map6);
			champ_destroy(&map7);
		}

		champ_destroy(&map0);
	}

	GIVEN("A large set of partially duplicate words") {
		std::ifstream lorem_ipsum_words("lorem_ipsum_words", std::ios::in | std::ios::binary);

		size_t data_size;
		lorem_ipsum_words.seekg(0, std::ios::end);
		data_size = lorem_ipsum_words.tellg();
		lorem_ipsum_words.seekg(0, std::ios::beg);

		std::unique_ptr<char []> data(new char [data_size]);
		lorem_ipsum_words.read(data.get(), data_size);
		int words_idx = 0;
		std::unique_ptr<char *[]> words(new char *[400]);

		auto map = champ_new(hash_mock, equals_mock);

		WHEN("The map is used to count word frequencies") {

			for (size_t i = 0, start = 0; i < data_size; ++i)
			{
				if (data[i] == '\n') // End of line, got string
				{
					data[i] = 0;
					char *word = data.get() + start;
					words[words_idx++] = word;
					start = i + 1;

					long count = 1 + (long)champ_get(map, word, nullptr);

					auto tmp = champ_set(map, word, (int*)count, nullptr);
					champ_destroy(&map);
					map = tmp;
				}
			}

			THEN("it should contain all words") {
				REQUIRE(champ_length(map) == 214); // determined using `sort <lorem_ipsum_words | uniq | wc -l`
			}
		}

		WHEN("The map is used to count word frequencies using 'assoc'") {
			auto assocfn = [](const char *key, const int *val, void *) {
				long count = (long)val;
				count += 1;
				return (int *)count;
			};

			for (size_t i = 0, start = 0; i < data_size; ++i)
			{
				if (data[i] == '\n') // End of line, got string
				{
					data[i] = 0;
					char *word = data.get() + start;
					words[words_idx++] = word;
					start = i + 1;

					auto tmp = champ_assoc(map, word, assocfn, nullptr);
					champ_destroy(&map);
					map = tmp;
				}
			}

			THEN("it should contain all words") {
				REQUIRE(champ_length(map) == 214);
			}
		}

		champ_destroy(&map);
	}

	GIVEN("A map with two deeply nested nodes") {
		auto char2int_hash = [](const char *key) {
			uint32_t i = *(const int *)key;
			return i;
		};
		auto char2int_equals = [](const char *l, const char *r) {
			uint32_t li = *(const int *)l;
			uint32_t ri = *(const int *)r;
			return (int)(li == ri);
		};
		std::function<int(struct node *, struct node **)> get_nodes = [&](struct node *node, struct node *buffer[]) {
			struct node **branches = ((struct node **)(node->content + node->element_arity));
			int index = 0;
			for (int b = 0; b < node->branch_arity; ++b) {
				buffer[index++] = branches[b];
				index += get_nodes(branches[b], buffer + index);
			}
			return index;
		};
		int ints[] = {
			0b01000000000000000,
			0b10000000000000000,
			0b11000000000000000,
		};
		const int *values[] = {
			&ints[0],
			&ints[1],
			&ints[2],
		};
		const char *keys[] = {
			(const char *)&ints[0],
			(const char *)&ints[1],
			(const char *)&ints[2],
		};
		auto map = champ_of(char2int_hash, char2int_equals, (char **)keys, (int **)values, 2);

		THEN("The values should all be reachable") {
			REQUIRE(champ_get(map, keys[0], nullptr) == values[0]);
			REQUIRE(champ_get(map, keys[1], nullptr) == values[1]);
		}

		WHEN("Inserting an equally deeply nested key") {

			THEN("Exactly that many nodes are created and deleted") {
				struct node *before[10];
				before[0] = (struct node *)map->root;
				int before_count = 1;
				before_count += get_nodes((struct node *)map->root, (struct node **)before + 1);

				auto tmp = champ_set(map, keys[2], values[2], nullptr);

				struct node *after[10];
				after[0] = (struct node *)tmp->root;
				int after_count = 1;
				after_count += get_nodes((struct node *)tmp->root, (struct node **)after + 1);

				REQUIRE(before_count == 4);
				REQUIRE(after_count == 4);

				REQUIRE(before[0] != after[0]);
				REQUIRE(before[1] != after[1]);
				REQUIRE(before[2] != after[2]);
				REQUIRE(before[3] != after[3]);

				REQUIRE(before[0]->ref_count == 1);
				REQUIRE(before[1]->ref_count == 1);
				REQUIRE(before[2]->ref_count == 1);
				REQUIRE(before[3]->ref_count == 1);

				champ_destroy(&map);
				map = tmp;

				REQUIRE(after[0]->ref_count == 1);
				REQUIRE(after[1]->ref_count == 1);
				REQUIRE(after[2]->ref_count == 1);
				REQUIRE(after[3]->ref_count == 1);
			}
		}

		WHEN("Removing one of those values") {
			int modified = 0;
			auto tmp = champ_del(map, keys[0], &modified);

			THEN("The other one should be pulled up") {
				REQUIRE(tmp->root->branch_arity == 0);
				REQUIRE(tmp->root->branch_map == 0);

				REQUIRE(champ_length(tmp) == 1);
				REQUIRE(champ_get(tmp, keys[1], nullptr) == values[1]);
			}

			champ_destroy(&tmp);
		}

		WHEN("Trying to remove an entry that would be deeply nested but is not present") {
			int modified = 0;
			auto tmp = champ_del(map, keys[2], &modified);

			THEN("The \"modified\" flag should be false") {
				REQUIRE(modified == 0);
			}

			THEN("The returned map should be the exact same map") {
				REQUIRE(tmp == map);
				REQUIRE(tmp->root == map->root);
			}

			THEN("The old map should be unchanged") {
				REQUIRE(champ_length(map) == 2);
				REQUIRE(champ_get(map, keys[0], nullptr) == values[0]);
				REQUIRE(champ_get(map, keys[1], nullptr) == values[1]);
			}
		}

		WHEN("First adding a top level element and then removing a deeply nested element") {
			int hash = 0b00001;
			const char *key = (const char *)&hash;
			int *value = &hash;

			auto tmp = champ_set(map, key, value, nullptr);
			auto tmp2 = champ_del(tmp, keys[1], nullptr);
			champ_destroy(&tmp);
			tmp = tmp2;

			THEN("The deeply nested element should have been inlined together with the top level element") {
				REQUIRE(champ_length(tmp) == 2);
				REQUIRE(champ_get(tmp, key, nullptr) == value);
				REQUIRE(champ_get(tmp, keys[0], nullptr) == values[0]);

				REQUIRE(tmp->root->branch_map == 0);
				REQUIRE(tmp->root->element_arity == 2);
			}

			champ_destroy(&tmp);
		}

		WHEN("First adding a third deeply nested element and then removing one") {
			auto tmp = champ_set(map, keys[2], values[2], nullptr);
			auto tmp2 = champ_del(tmp, keys[1], nullptr);
			REQUIRE(tmp2 != tmp);
			champ_destroy(&tmp);
			tmp = tmp2;

			THEN("The deeply nested elements should stay deeply nested") {
				REQUIRE(champ_length(tmp) == 2);
				REQUIRE(champ_get(tmp, keys[0], nullptr) == values[0]);
				REQUIRE(champ_get(tmp, keys[2], nullptr) == values[2]);

				REQUIRE(tmp->root->branch_map == 0b1);
				REQUIRE(tmp->root->branch_arity == 1);
			}

			champ_destroy(&tmp);
		}

		WHEN("First adding a third deeply nested element and a fourth, slightly less deeply nested element and then removing a deeply nested one") {
			int hash = 0b10000000000;
			const char *key = (const char *)&hash;
			int *value = &hash;

			auto tmp = champ_set(map, keys[2], values[2], nullptr);
			auto tmp2 = champ_set(tmp, key, value, nullptr);
			champ_destroy(&tmp);
			tmp = champ_del(tmp2, keys[0], nullptr);
			champ_destroy(&tmp2);

			THEN("The slightly less deeply nested node should be updated with the deeply nested node as a branch") {
				REQUIRE(champ_length(tmp) == 3);
				REQUIRE(champ_get(tmp, keys[1], nullptr) == values[1]);
				REQUIRE(champ_get(tmp, keys[2], nullptr) == values[2]);
				REQUIRE(champ_get(tmp, key, nullptr) == value);
			}

			champ_destroy(&tmp);
		}

		champ_destroy(&map);
	}
	
	GIVEN("A map with a few values") {
		const int l = 7;
		const char *keys[l] = {"a", "b", "c", "d", "e", "f", "g"};
		const int values[l] = {321, 432, 543, 654, 765, 876, 987};

		auto map = champ_new(hash_mock, equals_mock);
		for (int i = 0; i < l; ++i) {
			auto tmp = champ_set(map, keys[i], &values[i], nullptr);
			champ_destroy(&map);
			map = tmp;
		}
		
		WHEN("An entry is removed") {
			int modified = 0;
			auto tmp = champ_del(map, "d", &modified);
			
			THEN("The \"modified\" flag should be true") {
				REQUIRE(modified == 1);
			}
			
			THEN("The new map should no longer contain the removed key") {
				REQUIRE(champ_get(tmp, "d", nullptr) == nullptr);
			}
			
			THEN("The new map should still contain all other keys") {
				REQUIRE(*champ_get(tmp, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(tmp, keys[1], nullptr) == values[1]);
				REQUIRE(*champ_get(tmp, keys[2], nullptr) == values[2]);
				REQUIRE(*champ_get(tmp, keys[4], nullptr) == values[4]);
				REQUIRE(*champ_get(tmp, keys[5], nullptr) == values[5]);
				REQUIRE(*champ_get(tmp, keys[6], nullptr) == values[6]);
			}
			
			THEN("The old map should be unchanged") {
				REQUIRE(champ_length(map) == 7);
				REQUIRE(*champ_get(map, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(map, keys[1], nullptr) == values[1]);
				REQUIRE(*champ_get(map, keys[2], nullptr) == values[2]);
				REQUIRE(*champ_get(map, keys[3], nullptr) == values[3]);
				REQUIRE(*champ_get(map, keys[4], nullptr) == values[4]);
				REQUIRE(*champ_get(map, keys[5], nullptr) == values[5]);
				REQUIRE(*champ_get(map, keys[6], nullptr) == values[6]);
			}
			
			THEN("The new length should be smaller") {
				int new_length = champ_length(tmp);
				int old_length = champ_length(map);
				REQUIRE(new_length == old_length - 1);
			}

			champ_destroy(&tmp);
		}

		WHEN("Trying to remove an entry that is not present") {
			int modified = 0;
			auto tmp = champ_del(map, "something completely different", &modified);

			THEN("The \"modified\" flag should be false") {
				REQUIRE(modified == 0);
			}

			THEN("The returned map should be the exact same pointer") {
				REQUIRE(tmp == map);
			}

			THEN("The old map should be unchanged") {
				REQUIRE(champ_length(map) == 7);
				REQUIRE(*champ_get(map, keys[0], nullptr) == values[0]);
				REQUIRE(*champ_get(map, keys[1], nullptr) == values[1]);
				REQUIRE(*champ_get(map, keys[2], nullptr) == values[2]);
				REQUIRE(*champ_get(map, keys[3], nullptr) == values[3]);
				REQUIRE(*champ_get(map, keys[4], nullptr) == values[4]);
				REQUIRE(*champ_get(map, keys[5], nullptr) == values[5]);
				REQUIRE(*champ_get(map, keys[6], nullptr) == values[6]);
			}
		}
		
		champ_destroy(&map);
	}

	GIVEN("A map with exactly two values") {
		const int a = 321, b = 432;

		auto map0 = champ_new(hash_mock, equals_mock);
		
		auto map1 = champ_set(map0, "a", &a, nullptr);
		auto map2 = champ_set(map1, "b", &b, nullptr);
		champ_destroy(&map1);

		WHEN("A single entry is removed") {
			auto single = champ_del(map2, "a", nullptr);

			THEN("The new map should be in a valid state") {
				REQUIRE(champ_length(single) == 1);
				int found;
				const int *value = champ_get(single, "b", &found);
				REQUIRE(found == 1);
				REQUIRE(*value == b);
			}

			champ_destroy(&single);
		}

		WHEN("Both entries are removed") {
			int modified = 0;

			auto tmp = champ_del(map2, "a", nullptr);
			auto empty = champ_del(tmp, "b", nullptr);
			champ_destroy(&tmp);

			THEN("The new map should be empty") {
				int found;

				REQUIRE(champ_get(empty, "a", &found) == nullptr);
				REQUIRE(found == 0);

				REQUIRE(champ_get(empty, "b", &found) == nullptr);
				REQUIRE(found == 0);

				REQUIRE(champ_length(empty) == 0);
			}

			THEN("The newly empty map and the original empty map should have the identical root node") {
				REQUIRE(empty->root == map0->root);
			}

			champ_destroy(&empty);
		}

		champ_destroy(&map0);
		champ_destroy(&map2);
	}

	GIVEN("A map with a enough values to require nested nodes") {
		const char *keys[52] = {
			"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
			"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
			"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
			"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
		};

		auto map = champ_new(hash_mock, equals_mock);
		for (long i = 0; i < 52; ++i) {
			auto tmp = champ_set(map, keys[i], (int *)i, nullptr);
			champ_destroy(&map);
			map = tmp;
		}

		WHEN("Iterating over the map") {
			struct champ_iter iterator;
			champ_iter_init(&iterator, map);
			const char *key;
			long value;

			THEN("The iterator should yield exactly the correct amount of entries") {
				int yield_count = 0;

				for (;champ_iter_next(&iterator, (char **)&key, (int **)&value);) {
					++yield_count;
				}

				REQUIRE(yield_count == 52);
			}

			THEN("The iterator should yield each entry") {
				std::map<const char *,int> yields;

				for (;champ_iter_next(&iterator, (char **)&key, (int **)&value);) {
					yields[key] = value;
				}

				for (long i = 0; i < 52; ++i) {
					key = keys[i];
					value = i;
					REQUIRE(yields.find(key) != yields.end());
					REQUIRE(yields[key] == value);
				}
			}

			THEN("The iterator should yield each entry only once") {
				std::map<const char *,int> counts;

				for (;champ_iter_next(&iterator, (char **)&key, (int **)&value);) {
					counts[key] = 1 + counts[key];
				}

				for (auto it = counts.begin(); it != counts.end(); ++it) {
					REQUIRE(it->second == 1);
				}
			}
		}

		champ_destroy(&map);
	}

	GIVEN("A set of keys with colliding hashes") {
#define HASH(p30, p25, p20, p15, p10, p5, p0) (uint32_t)0b##p30##p25##p20##p15##p10##p5##p0##u

		auto hash = [](const char *key) {
			std::string skey {key};
			if (skey == "c1") return HASH(00, 00000, 00000, 00000, 00000, 00000, 00000);
			if (skey == "c2") return HASH(00, 00000, 00000, 00000, 00000, 00000, 00000);
			if (skey == "c3") return HASH(00, 00000, 00000, 00000, 00000, 00000, 00000);
			if (skey == "n1") return HASH(01, 00000, 00000, 00000, 00000, 00000, 00000);
			if (skey == "n1") return HASH(10, 00000, 00000, 00000, 00000, 00000, 00000);
			if (skey == "n2") return HASH(11, 00000, 00000, 00000, 00000, 00000, 00000);
		};
		auto equals = [](const char *l, const char *r) {
			std::string sl {l}, sr {r};
			return (int)(sl == sr);
		};

		const char *keys[] = {"c1", "c2", "c3", "n1", "n2", "n3"};
		int *values[] = {(int *)1, (int *)2, (int *)3, (int *)4, (int *)5, (int *)6};

		auto map = champ_new(hash, equals);

		WHEN("Inserting two colliding entries") {
			int replaced1 = 1, replaced2 = 1;
			auto tmp1 = champ_set(map, keys[0], values[0], &replaced1);
			auto tmp2 = champ_set(tmp1, keys[1], values[1], &replaced2);

			THEN("They can be retrieved") {
				int found1 = 0, found2 = 0;
				REQUIRE(champ_get(tmp2, keys[0], &found1) == values[0]);
				REQUIRE(champ_get(tmp2, keys[1], &found2) == values[1]);
				REQUIRE(champ_get(tmp2, keys[0], nullptr) == values[0]);
				REQUIRE(champ_get(tmp2, keys[1], nullptr) == values[1]);
				REQUIRE(found1 == 1);
				REQUIRE(found2 == 1);
				REQUIRE(replaced1 == 0);
				REQUIRE(replaced2 == 0);
				REQUIRE(champ_length(tmp2) == 2);
			}

			THEN("The containing node is pushed all the way down") {
				// todo: no good way to test this yet
				REQUIRE(tmp2->root->branch_arity == 1);
				REQUIRE(tmp2->root->element_arity == 0);
			}

			THEN("A nonexistant entry with a colliding hash can not be found") {
				int found = 1;
				REQUIRE(champ_get(tmp2, keys[2], &found) == nullptr);
				REQUIRE(champ_get(tmp2, keys[2], nullptr) == nullptr);
				REQUIRE(found == 0);
			}

			THEN("A nonexistant entry with a colliding hash can not be removed") {
				int found = 1;
				auto tmp3 = champ_del(tmp2, keys[2], &found);
				REQUIRE(tmp3 == tmp2);
				REQUIRE(found == 0);
			}

			WHEN("And deleting one of them again") {
				int was_deleted = 0;
				auto tmp3 = champ_del(tmp2, keys[0], &was_deleted);

				THEN("The remaining value can be retrieved") {
					int found = 0;
					REQUIRE(champ_get(tmp3, keys[1], &found) == values[1]);
					REQUIRE(champ_get(tmp3, keys[1], nullptr) == values[1]);
					REQUIRE(found == 1);
				}

				THEN("The deleted value is removed") {
					int found = 1;
					REQUIRE(champ_get(tmp3, keys[0], &found) == nullptr);
					REQUIRE(champ_get(tmp3, keys[0], nullptr) == nullptr);
					REQUIRE(found == 0);
					REQUIRE(was_deleted == 1);
					REQUIRE(champ_length(tmp3) == 1);
				}

				THEN("The containing node is pulled all the way up again") {
					REQUIRE(tmp3->root->branch_arity == 0);
					REQUIRE(tmp3->root->element_arity == 1);
				}

				THEN("The previous map is untouched") {
					int found1 = 0, found2 = 0;
					REQUIRE(champ_get(tmp2, keys[0], &found1) == values[0]);
					REQUIRE(champ_get(tmp2, keys[1], &found2) == values[1]);
					REQUIRE(champ_get(tmp2, keys[0], nullptr) == values[0]);
					REQUIRE(champ_get(tmp2, keys[1], nullptr) == values[1]);
					REQUIRE(found1 == 1);
					REQUIRE(found2 == 1);
					REQUIRE(champ_length(tmp2) == 2);
				}

				champ_destroy(&tmp3);
			}

			WHEN("Updating a colliding entry") {
				int replaced = 0;
				auto tmp3 = champ_set(tmp2, keys[0], values[2], &replaced);

				THEN("The key should have been updated") {
					int found = 0;
					REQUIRE(champ_get(tmp3, keys[0], &found) == values[2]);
					REQUIRE(champ_get(tmp3, keys[0], nullptr) == values[2]);
					REQUIRE(found == 1);
					REQUIRE(replaced == 1);
					REQUIRE(champ_length(tmp3) == 2);
				}

				THEN("The other key should still be the same") {
					int found = 0;
					REQUIRE(champ_get(tmp3, keys[1], &found) == values[1]);
					REQUIRE(champ_get(tmp3, keys[1], nullptr) == values[1]);
					REQUIRE(found == 1);
				}

				THEN("The previous map is untouched") {
					int found1 = 0, found2 = 0;
					REQUIRE(champ_get(tmp2, keys[0], &found1) == values[0]);
					REQUIRE(champ_get(tmp2, keys[1], &found2) == values[1]);
					REQUIRE(champ_get(tmp2, keys[0], nullptr) == values[0]);
					REQUIRE(champ_get(tmp2, keys[1], nullptr) == values[1]);
					REQUIRE(found1 == 1);
					REQUIRE(found2 == 1);
					REQUIRE(champ_length(tmp2) == 2);
				}

				champ_destroy(&tmp3);
			}

			champ_destroy(&tmp1);
			DEBUG_NOTICE("next destroy should remove %d nodes\n", 8);
			champ_destroy(&tmp2);
		}

		WHEN("Inserting three colliding entries") {
			auto tmp1 = champ_set(map, keys[0], values[0], nullptr);
			auto tmp2 = champ_set(tmp1, keys[1], values[1], nullptr);
			auto tmp3 = champ_set(tmp2, keys[2], values[2], nullptr);

			THEN("They can be retrieved") {
				int found1 = 0, found2 = 0, found3;
				REQUIRE(champ_get(tmp3, keys[0], &found1) == values[0]);
				REQUIRE(champ_get(tmp3, keys[1], &found2) == values[1]);
				REQUIRE(champ_get(tmp3, keys[2], &found3) == values[2]);
				REQUIRE(champ_get(tmp3, keys[0], nullptr) == values[0]);
				REQUIRE(champ_get(tmp3, keys[1], nullptr) == values[1]);
				REQUIRE(champ_get(tmp3, keys[2], nullptr) == values[2]);
				REQUIRE(found1 == 1);
				REQUIRE(found2 == 1);
				REQUIRE(found3 == 1);
				REQUIRE(champ_length(tmp3) == 3);
			}

			WHEN("And deleting one of them") {
				int was_deleted = 0;
				auto tmp4 = champ_del(tmp3, keys[0], &was_deleted);

				THEN("The remaining values can be retrieved") {
					int found1 = 0, found2 = 0;
					REQUIRE(champ_get(tmp4, keys[1], &found1) == values[1]);
					REQUIRE(champ_get(tmp4, keys[1], nullptr) == values[1]);
					REQUIRE(found1 == 1);
					REQUIRE(champ_get(tmp4, keys[2], &found2) == values[2]);
					REQUIRE(champ_get(tmp4, keys[2], nullptr) == values[2]);
					REQUIRE(found2 == 1);
				}

				THEN("The deleted value is removed") {
					int found = 1;
					REQUIRE(champ_get(tmp4, keys[0], &found) == nullptr);
					REQUIRE(champ_get(tmp4, keys[0], nullptr) == nullptr);
					REQUIRE(found == 0);
					REQUIRE(was_deleted == 1);
					REQUIRE(champ_length(tmp4) == 2);
				}

				THEN("The containing node is still all the way down") {
					REQUIRE(tmp3->root->branch_arity == 1);
					REQUIRE(tmp3->root->element_arity == 0);
				}

				THEN("The previous map is untouched") {
					int found1 = 0, found2 = 0, found3;
					REQUIRE(champ_get(tmp3, keys[0], &found1) == values[0]);
					REQUIRE(champ_get(tmp3, keys[1], &found2) == values[1]);
					REQUIRE(champ_get(tmp3, keys[2], &found3) == values[2]);
					REQUIRE(champ_get(tmp3, keys[0], nullptr) == values[0]);
					REQUIRE(champ_get(tmp3, keys[1], nullptr) == values[1]);
					REQUIRE(champ_get(tmp3, keys[2], nullptr) == values[2]);
					REQUIRE(found1 == 1);
					REQUIRE(found2 == 1);
					REQUIRE(found3 == 1);
					REQUIRE(champ_length(tmp3) == 3);
				}

				champ_destroy(&tmp4);
			}

			champ_destroy(&tmp1);
			champ_destroy(&tmp2);
			champ_destroy(&tmp3);
		}

		WHEN("Inserting values using 'assoc'") {
			int found_hist[100];
			int found_hist_i = -1;
			const char *current_assoc_key;
			auto assocfn = [&](const char *key, const int *value) {
				int found = 0 != (long)value ? 1 : 0;
				found_hist[++found_hist_i] = found;
				if (!key) key = current_assoc_key;
				for (unsigned i = 0; i < 6; ++i)
					if (std::string(keys[i]) == std::string(key))
						return (int *)(100 * found + (long)values[i]);
				return (int *)nullptr;
			};
			auto apply = [](const char *key, const int *value, void *callback) {
				return (*static_cast<decltype(assocfn) *>(callback))(key, value);
			};

			int replaced1 = 1, replaced2 = 1;
			current_assoc_key = keys[0];
			auto tmp1 = champ_assoc(map, keys[0], apply, &assocfn);
			replaced1 = found_hist[found_hist_i];
			current_assoc_key = keys[1];
			auto tmp2 = champ_assoc(tmp1, keys[1], apply, &assocfn);
			replaced2 = found_hist[found_hist_i];

			THEN("They can be retrieved") {
				int found1 = 0, found2 = 0;
				REQUIRE(champ_get(tmp2, keys[0], &found1) == values[0]);
				REQUIRE(champ_get(tmp2, keys[1], &found2) == values[1]);
				REQUIRE(champ_get(tmp2, keys[0], nullptr) == values[0]);
				REQUIRE(champ_get(tmp2, keys[1], nullptr) == values[1]);
				REQUIRE(found1 == 1);
				REQUIRE(found2 == 1);
				REQUIRE(replaced1 == 0);
				REQUIRE(replaced2 == 0);
				REQUIRE(champ_length(tmp2) == 2);
			}

			THEN("The containing node is pushed all the way down") {
				// todo: no good way to test this yet
				REQUIRE(tmp2->root->branch_arity == 1);
				REQUIRE(tmp2->root->element_arity == 0);
			}

			THEN("A nonexistant entry with a colliding hash can not be found") {
				int found = 1;
				REQUIRE(champ_get(tmp2, keys[2], &found) == nullptr);
				REQUIRE(champ_get(tmp2, keys[2], nullptr) == nullptr);
				REQUIRE(found == 0);
			}

			THEN("A nonexistant entry with a colliding hash can not be removed") {
				int found = 1;
				auto tmp3 = champ_del(tmp2, keys[2], &found);
				REQUIRE(tmp3 == tmp2);
				REQUIRE(found == 0);
			}

			WHEN("And deleting one of them again") {
				int was_deleted = 0;
				auto tmp3 = champ_del(tmp2, keys[0], &was_deleted);

				THEN("The remaining value can be retrieved") {
					int found = 0;
					REQUIRE(champ_get(tmp3, keys[1], &found) == values[1]);
					REQUIRE(champ_get(tmp3, keys[1], nullptr) == values[1]);
					REQUIRE(found == 1);
				}

				THEN("The deleted value is removed") {
					int found = 1;
					REQUIRE(champ_get(tmp3, keys[0], &found) == nullptr);
					REQUIRE(champ_get(tmp3, keys[0], nullptr) == nullptr);
					REQUIRE(found == 0);
					REQUIRE(was_deleted == 1);
					REQUIRE(champ_length(tmp3) == 1);
				}

				THEN("The containing node is pulled all the way up again") {
					REQUIRE(tmp3->root->branch_arity == 0);
					REQUIRE(tmp3->root->element_arity == 1);
				}

				THEN("The previous map is untouched") {
					int found1 = 0, found2 = 0;
					REQUIRE(champ_get(tmp2, keys[0], &found1) == values[0]);
					REQUIRE(champ_get(tmp2, keys[1], &found2) == values[1]);
					REQUIRE(champ_get(tmp2, keys[0], nullptr) == values[0]);
					REQUIRE(champ_get(tmp2, keys[1], nullptr) == values[1]);
					REQUIRE(found1 == 1);
					REQUIRE(found2 == 1);
					REQUIRE(champ_length(tmp2) == 2);
				}

				champ_destroy(&tmp3);
			}

			WHEN("Updating a colliding entry") {
				int replaced = 0;
				auto tmp3 = champ_assoc(tmp2, keys[0], apply, &assocfn);
				replaced = found_hist[found_hist_i];

				THEN("The key should have been updated") {
					int found = 0;
					REQUIRE((long)champ_get(tmp3, keys[0], &found) == 100 + (long)values[0]);
					REQUIRE((long)champ_get(tmp3, keys[0], nullptr) == 100 + (long)values[0]);
					REQUIRE(found == 1);
					REQUIRE(replaced == 1);
					REQUIRE(champ_length(tmp3) == 2);
				}

				THEN("The other key should still be the same") {
					int found = 0;
					REQUIRE(champ_get(tmp3, keys[1], &found) == values[1]);
					REQUIRE(champ_get(tmp3, keys[1], nullptr) == values[1]);
					REQUIRE(found == 1);
				}

				THEN("The previous map is untouched") {
					int found1 = 0, found2 = 0;
					REQUIRE(champ_get(tmp2, keys[0], &found1) == values[0]);
					REQUIRE(champ_get(tmp2, keys[1], &found2) == values[1]);
					REQUIRE(champ_get(tmp2, keys[0], nullptr) == values[0]);
					REQUIRE(champ_get(tmp2, keys[1], nullptr) == values[1]);
					REQUIRE(found1 == 1);
					REQUIRE(found2 == 1);
					REQUIRE(champ_length(tmp2) == 2);
				}

				champ_destroy(&tmp3);
			}

			champ_destroy(&tmp1);
			DEBUG_NOTICE("next destroy should remove %d nodes\n", 8);
			champ_destroy(&tmp2);
		}

		champ_destroy(&map);
	}

	GIVEN("Two maps") {
		auto l0 = champ_new(hash_mock, equals_mock);
		auto r0 = champ_new(hash_mock, equals_mock);
		auto value_equals = [](const int *l, const int *r) {
			return (int)(l == r);
		};
		auto value_equals_raise = [](const int *l, const int *r) {
			throw "Compare function should not be called";
			return (int)1;
		};


		WHEN("They are empty") {
			THEN("They should be equal") {
				REQUIRE(champ_equals(l0, r0, value_equals) == 1);
				REQUIRE(champ_equals(r0, l0, value_equals) == 1);
			}
		}

		WHEN("One is empty, one is not") {
			auto l1 = champ_set(l0, "foo", (int *)1, nullptr);

			THEN("They should not be equal") {
				REQUIRE_FALSE(champ_equals(l1, r0, value_equals));
				REQUIRE_FALSE(champ_equals(r0, l1, value_equals));
			}

			THEN("Equality should short-circuit") {
				REQUIRE_FALSE(champ_equals(l1, r0, value_equals_raise));
				REQUIRE_FALSE(champ_equals(r0, l1, value_equals_raise));
			}

			champ_destroy(&l1);
		}

		WHEN("Both contain the same entry") {
			auto l1 = champ_set(l0, "foo", (int *)1, nullptr);
			auto r1 = champ_set(r0, "foo", (int *)1, nullptr);

			THEN("They should be equal") {
				REQUIRE(champ_equals(l1, r1, value_equals));
				REQUIRE(champ_equals(r1, l1, value_equals));
			}

			THEN("Identity should short-circuit") {
				REQUIRE(champ_equals(l1, l1, value_equals_raise));
			}

			champ_destroy(&l1);
			champ_destroy(&r1);
		}

		WHEN("They have equal keys mapping to separate values") {
			auto l1 = champ_set(l0, "foo", (int *)1, nullptr);
			auto r1 = champ_set(r0, "foo", (int *)2, nullptr);

			THEN("They should not be equal") {
				REQUIRE_FALSE(champ_equals(l1, r1, value_equals));
				REQUIRE_FALSE(champ_equals(r1, l1, value_equals));
			}

			champ_destroy(&l1);
			champ_destroy(&r1);
		}

		WHEN("Inserting partially hash-colliding entries") {
			auto hash = [](const char *s) {
				return (uint32_t)(long)s;
			};
			auto equals = [](const char *l, const char *r) {
				return (int)(l == r);
			};

			char *h0 = (char *)HASH(00, 00000, 00000, 00000, 00000, 00000, 00000);
			char *h1 = (char *)HASH(00, 00000, 00000, 00000, 00001, 00000, 00000);
			char *h2 = (char *)HASH(00, 00000, 00000, 00000, 00010, 00000, 00000);

			auto l0 = champ_new(hash, equals);
			auto r0 = champ_new(hash, equals);

			auto l1 = champ_set(l0, h0, (int *)1, nullptr);

			auto r1 = champ_set(r0, h1, (int *)2, nullptr);
			auto r2 = champ_set(r1, h0, (int *)1, nullptr);

			THEN("Equal hash prefixes for elements and branches should not imply equality") {
				REQUIRE_FALSE(champ_equals(l1, r2, value_equals));
				REQUIRE_FALSE(champ_equals(r2, l1, value_equals));
			}

			THEN("Unequal element_maps and branch_maps should short-circuit") {
				REQUIRE_FALSE(champ_equals(l1, r2, value_equals_raise));
				REQUIRE_FALSE(champ_equals(r2, l1, value_equals_raise));
			}

			auto r3 = champ_del(r2, h1, nullptr);

			THEN("Deleting entries until equal should still produce equal maps") {
				REQUIRE(champ_equals(r3, l1, value_equals));
				REQUIRE(champ_equals(l1, r3, value_equals));
			}

			auto l2 = champ_set(l1, h1, (int *)3, nullptr);

			THEN("Equal branch_maps should not imply equality") {
				REQUIRE_FALSE(champ_equals(l2, r2, value_equals));
				REQUIRE_FALSE(champ_equals(r2, l2, value_equals));
			}

			champ_destroy(&l0);
			champ_destroy(&l1);
			champ_destroy(&l2);
			champ_destroy(&r0);
			champ_destroy(&r1);
			champ_destroy(&r2);
			champ_destroy(&r3);
		}


		WHEN("They arrive at the same state independently") {
			std::ifstream lorem_ipsum_words("lorem_ipsum_words", std::ios::in | std::ios::binary);

			size_t data_size;
			lorem_ipsum_words.seekg(0, std::ios::end);
			data_size = lorem_ipsum_words.tellg();
			lorem_ipsum_words.seekg(0, std::ios::beg);

			std::unique_ptr<char []> data(new char [data_size]);
			lorem_ipsum_words.read(data.get(), data_size);
			int words_idx = 0;
			std::unique_ptr<char *[]> words(new char *[400]);

			auto l = champ_new(hash_mock, equals_mock);
			auto r = champ_new(hash_mock, equals_mock);


			for (size_t i = 0, start = 0; i < data_size; ++i)
			{
				if (data[i] == '\n') // End of line, got string
				{
					data[i] = 0;
					char *word = data.get() + start;
					words[words_idx++] = word;
					start = i + 1;

					long lcount = 1 + (long)champ_get(l, word, nullptr);
					long rcount = 1 + (long)champ_get(r, word, nullptr);

					struct champ *tmp;

					tmp = champ_set(l, word, (int*)lcount, nullptr);
					champ_destroy(&l);
					l = tmp;

					tmp = champ_set(r, word, (int*)rcount, nullptr);
					champ_destroy(&r);
					r = tmp;
				}
			}

			THEN("They should be equal") {
				REQUIRE(champ_equals(l, r, value_equals));
				REQUIRE(champ_equals(r, l, value_equals));
			}

			THEN("Removing an entry and adding it should produce equal maps") {
				struct champ_iter it;
				champ_iter_init(&it, r);

				char *key;
				int *value;
				champ_iter_next(&it, &key, &value);
				champ_iter_next(&it, &key, &value);
				champ_iter_next(&it, &key, &value);
				champ_iter_next(&it, &key, &value);

				auto r1 = champ_del(r, key, nullptr);
				auto r2 = champ_set(r1, key, value, nullptr);

				REQUIRE(champ_equals(r2, l, value_equals));
				REQUIRE(champ_equals(l, r2, value_equals));

				champ_destroy(&r1);
				champ_destroy(&r2);
			}

			champ_destroy(&l);
			champ_destroy(&r);
		}

		WHEN("Inserting values in opposite order") {
			const char *keys[52] = {
				"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
				"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
				"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
				"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
			};

			auto r1 = champ_set(r0, "answer", (int *)42, nullptr);
			auto l1 = champ_set(l0, "answer", (int *)42, nullptr);

			for (long i = 0; i < 52; ++i) {
				struct champ *tmp;

				tmp = champ_set(l1, keys[i], (int *)i, nullptr);
				champ_destroy(&l1);
				l1 = tmp;

				tmp = champ_set(r1, keys[51 - i], (int *)(51 - i), nullptr);
				champ_destroy(&r1);
				r1 = tmp;
			}

			THEN("Equal maps should still be equal") {
				REQUIRE(champ_equals(r1, l1, value_equals));
				REQUIRE(champ_equals(l1, r1, value_equals));
			}

			champ_destroy(&l1);
			champ_destroy(&r1);
		}

		champ_destroy(&l0);
		champ_destroy(&r0);
	}
}
