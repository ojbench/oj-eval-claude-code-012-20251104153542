/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */

template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;

private:
    struct Node {
        value_type data;
        Node* prev;
        Node* next;
        Node* hash_prev;
        Node* hash_next;

        Node(const value_type& d) : data(d), prev(nullptr), next(nullptr), hash_prev(nullptr), hash_next(nullptr) {}
    };

    Node* head;
    Node* tail;
    Node** hash_table;
    size_t table_size;
    size_t element_count;
    Hash hash_func;
    Equal equal_func;

    static const size_t INITIAL_SIZE = 16;

    void initialize_table(size_t size) {
        table_size = size;
        hash_table = new Node*[table_size];
        for (size_t i = 0; i < table_size; ++i) {
            hash_table[i] = nullptr;
        }
    }

    void clear_table() {
        if (hash_table) {
            for (size_t i = 0; i < table_size; ++i) {
                Node* current = hash_table[i];
                while (current) {
                    Node* next = current->hash_next;
                    delete current;
                    current = next;
                }
            }
            delete[] hash_table;
            hash_table = nullptr;
        }
    }

    void rehash() {
        size_t new_size = table_size * 2;
        Node** new_table = new Node*[new_size];
        for (size_t i = 0; i < new_size; ++i) {
            new_table[i] = nullptr;
        }

        Node* current = head;
        while (current) {
            size_t index = hash_func(current->data.first) % new_size;
            current->hash_prev = nullptr;
            current->hash_next = new_table[index];
            if (new_table[index]) {
                new_table[index]->hash_prev = current;
            }
            new_table[index] = current;
            current = current->next;
        }

        delete[] hash_table;
        hash_table = new_table;
        table_size = new_size;
    }

    Node* find_node(const Key& key) const {
        size_t index = hash_func(key) % table_size;
        Node* current = hash_table[index];
        while (current) {
            if (equal_func(current->data.first, key)) {
                return current;
            }
            current = current->hash_next;
        }
        return nullptr;
    }

    void remove_from_list(Node* node) {
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            head = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        } else {
            tail = node->prev;
        }
    }

    void remove_from_hash(Node* node) {
        size_t index = hash_func(node->data.first) % table_size;
        if (node->hash_prev) {
            node->hash_prev->hash_next = node->hash_next;
        } else {
            hash_table[index] = node->hash_next;
        }
        if (node->hash_next) {
            node->hash_next->hash_prev = node->hash_prev;
        }
    }

public:
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
	class const_iterator;
	class iterator {
	public:
		Node* node;
		const linked_hashmap* map;

	public:
		// The following code is written for the C++ type_traits library.
		// Type traits is a C++ feature for describing certain properties of a type.
		// For instance, for an iterator, iterator::value_type is the type that the
		// iterator points to.
		// STL algorithms and containers may use these type_traits (e.g. the following
		// typedef) to work properly.
		// See these websites for more information:
		// https://en.cppreference.com/w/cpp/header/type_traits
		// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
		// About iterator_category: https://en.cppreference.com/w/cpp/iterator
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::output_iterator_tag;


		iterator() : node(nullptr), map(nullptr) {}
		iterator(Node* n, const linked_hashmap* m) : node(n), map(m) {}
		iterator(const iterator &other) : node(other.node), map(other.map) {}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
		    if (!node || !map) throw invalid_iterator();
		    iterator temp = *this;
		    node = node->next;
		    return temp;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
		    if (!node || !map) throw invalid_iterator();
		    node = node->next;
		    return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
		    if (!map) throw invalid_iterator();
		    iterator temp = *this;
		    if (node) {
		        node = node->prev;
		    } else {
		        node = map->tail;
		    }
		    return temp;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
		    if (!map) throw invalid_iterator();
		    if (node) {
		        node = node->prev;
		    } else {
		        node = map->tail;
		    }
		    return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
		    if (!node) throw invalid_iterator();
		    return node->data;
		}
		bool operator==(const iterator &rhs) const {
		    return node == rhs.node && map == rhs.map;
		}
		bool operator==(const const_iterator &rhs) const {
		    return node == rhs.node && map == rhs.map;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
		    return !(*this == rhs);
		}
		bool operator!=(const const_iterator &rhs) const {
		    return !(*this == rhs);
		}

		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {
		    if (!node) return nullptr;
		    return &(node->data);
		}

		friend class const_iterator;
	};

	class const_iterator {
	public:
		const Node* node;
		const linked_hashmap* map;

	public:
		const_iterator() : node(nullptr), map(nullptr) {}
		const_iterator(const Node* n, const linked_hashmap* m) : node(n), map(m) {}
		const_iterator(const const_iterator &other) : node(other.node), map(other.map) {}
		const_iterator(const iterator &other) : node(other.node), map(other.map) {}

		const_iterator operator++(int) {
		    if (!node || !map) throw invalid_iterator();
		    const_iterator temp = *this;
		    node = node->next;
		    return temp;
		}

		const_iterator & operator++() {
		    if (!node || !map) throw invalid_iterator();
		    node = node->next;
		    return *this;
		}

		const_iterator operator--(int) {
		    if (!map) throw invalid_iterator();
		    const_iterator temp = *this;
		    if (node) {
		        node = node->prev;
		    } else {
		        node = map->tail;
		    }
		    return temp;
		}

		const_iterator & operator--() {
		    if (!map) throw invalid_iterator();
		    if (node) {
		        node = node->prev;
		    } else {
		        node = map->tail;
		    }
		    return *this;
		}

		const value_type & operator*() const {
		    if (!node) throw invalid_iterator();
		    return node->data;
		}

		bool operator==(const const_iterator &rhs) const {
		    return node == rhs.node && map == rhs.map;
		}

		bool operator==(const iterator &rhs) const {
		    return node == rhs.node && map == rhs.map;
		}

		bool operator!=(const const_iterator &rhs) const {
		    return !(*this == rhs);
		}

		bool operator!=(const iterator &rhs) const {
		    return !(*this == rhs);
		}

		const value_type* operator->() const noexcept {
		    if (!node) return nullptr;
		    return &(node->data);
		}
	};

	/**
	 * TODO two constructors
	 */
	linked_hashmap() : head(nullptr), tail(nullptr), hash_table(nullptr), table_size(0), element_count(0) {
	    initialize_table(INITIAL_SIZE);
	}

	linked_hashmap(const linked_hashmap &other) : head(nullptr), tail(nullptr), hash_table(nullptr), table_size(0), element_count(0) {
	    initialize_table(other.table_size);
	    hash_func = other.hash_func;
	    equal_func = other.equal_func;

	    Node* current = other.head;
	    while (current) {
	        insert(current->data);
	        current = current->next;
	    }
	}

	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
	    if (this == &other) return *this;

	    clear();
	    clear_table();

	    initialize_table(other.table_size);
	    hash_func = other.hash_func;
	    equal_func = other.equal_func;

	    Node* current = other.head;
	    while (current) {
	        insert(current->data);
	        current = current->next;
	    }

	    return *this;
	}

	/**
	 * TODO Destructors
	 */
	~linked_hashmap() {
	    clear();
	    clear_table();
	}

	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
	    Node* node = find_node(key);
	    if (!node) throw index_out_of_bound();
	    return node->data.second;
	}

	const T & at(const Key &key) const {
	    Node* node = find_node(key);
	    if (!node) throw index_out_of_bound();
	    return node->data.second;
	}

	/**
	 * TODO
	 * access specified element
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
	    Node* node = find_node(key);
	    if (node) {
	        return node->data.second;
	    }

	    value_type new_pair(key, T());
	    insert(new_pair);
	    return find_node(key)->data.second;
	}

	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
	    return at(key);
	}

	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
	    return iterator(head, this);
	}

	const_iterator cbegin() const {
	    return const_iterator(head, this);
	}

	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
	    return iterator(nullptr, this);
	}

	const_iterator cend() const {
	    return const_iterator(nullptr, this);
	}

	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
	    return element_count == 0;
	}

	/**
	 * returns the number of elements.
	 */
	size_t size() const {
	    return element_count;
	}

	/**
	 * clears the contents
	 */
	void clear() {
	    Node* current = head;
	    while (current) {
	        Node* next = current->next;
	        delete current;
	        current = next;
	    }
	    head = nullptr;
	    tail = nullptr;
	    element_count = 0;

	    for (size_t i = 0; i < table_size; ++i) {
	        hash_table[i] = nullptr;
	    }
	}

	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion),
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
	    Node* existing = find_node(value.first);
	    if (existing) {
	        return pair<iterator, bool>(iterator(existing, this), false);
	    }

	    if (element_count >= table_size * 0.75) {
	        rehash();
	    }

	    Node* new_node = new Node(value);

	    // Add to linked list
	    if (!head) {
	        head = new_node;
	        tail = new_node;
	    } else {
	        tail->next = new_node;
	        new_node->prev = tail;
	        tail = new_node;
	    }

	    // Add to hash table
	    size_t index = hash_func(value.first) % table_size;
	    new_node->hash_next = hash_table[index];
	    if (hash_table[index]) {
	        hash_table[index]->hash_prev = new_node;
	    }
	    hash_table[index] = new_node;

	    element_count++;
	    return pair<iterator, bool>(iterator(new_node, this), true);
	}

	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
	    if (!pos.node || pos.map != this) throw invalid_iterator();

	    Node* node = pos.node;
	    remove_from_hash(node);
	    remove_from_list(node);
	    delete node;
	    element_count--;
	}

	/**
	 * Returns the number of elements with key
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const {
	    return find_node(key) ? 1 : 0;
	}

	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
	    Node* node = find_node(key);
	    return node ? iterator(node, this) : end();
	}

	const_iterator find(const Key &key) const {
	    Node* node = find_node(key);
	    return node ? const_iterator(node, this) : cend();
	}
};

}

#endif