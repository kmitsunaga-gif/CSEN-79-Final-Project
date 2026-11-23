// Kimi Notes 
/* 
vector<T> is a class template, the C++ compiler needs to see both the class 
declaration and the full function definitions at the moment 
you instantiate a type like vector<int> or vector<MyObject> in main.cpp. 
If you leave the definitions in a separate .cpp file, 
the linker will fail with undefined reference errors*/

#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <utility>

#ifdef USEDEBUG
#define Debug( x ) std::cout << x
#else
#define Debug( x )
#endif

namespace coen79_final
{
    template <typename T>
    class vector {
    public:
        typedef std::size_t size_type;
        typedef T value_type;
        static constexpr size_type NOT_FOUND = static_cast<size_type>(-1);

        // Constructors & Destructor
        vector(const size_type& initial_capacity = 2);
        vector(const vector<T>& src);
        vector(vector<T>&& src) noexcept;
        ~vector();

        // Assignment operators
        vector<T>& operator=(const vector<T>& rhs);
        vector<T>& operator=(vector<T>&& rhs) noexcept;

        // Modification member functions
        void reserve(size_type new_capacity);
        bool insert_element(const value_type& entry);
        bool erase_element(const value_type& item);
        void push_back(const value_type& entry);
        void pop_back();
        void clear() noexcept;

        // element access
        value_type& operator[](size_type index);
        const value_type& operator[](size_type index) const;
        value_type& at(size_type index);
        const value_type& at(size_type index) const;

        // Getters + constant member funcs
        size_type size() const noexcept;
        size_type capacity() const noexcept;
        bool empty() const noexcept;
        void print_elements() const;

        // Iterators 
        value_type* begin() noexcept;
        const value_type* begin() const noexcept;
        value_type* end() noexcept;
        const value_type* end() const noexcept;

    private:
        value_type* element_array; // Pointer to dynamic array of value_type objects
        size_type aloc_slots;      // Total allocated slots in element_array
        size_type used_slots;      // Number of active elements in element_array

        // Helper functions
        size_type search_element(const value_type& element) const;
    };

 // -------------- IMPLEMENTATION -------------- //


    template <typename T>
    vector<T>::vector(const size_type& initial_capacity) {
        
        used_slots = 0;
        aloc_slots = initial_capacity;
        element_array = new value_type[aloc_slots];
    }

    template <typename T>
    vector<T>::vector(const vector<T>& src) {

        Debug("Copy constructor..." << std::endl);
        used_slots = src.used_slots;
        aloc_slots = src.aloc_slots;
        element_array = new value_type[aloc_slots];

        for (size_type i = 0; i < used_slots; ++i) {
            element_array[i] = src.element_array[i];
        }
    }

    template <typename T>
    vector<T>::vector(vector<T>&& src) 
                                            // NOTE: was recommended to use no except here, make sure to understand how it works
    noexcept
        : element_array(src.element_array),
          aloc_slots(src.aloc_slots),
          used_slots(src.used_slots) {
    
        src.element_array = nullptr;
        src.aloc_slots = 0;
        src.used_slots = 0;
    }

    template <typename T>
    vector<T>::~vector() {

        delete[] element_array;
        element_array = nullptr;
        used_slots = 0;
        aloc_slots = 0;
    }

    template <typename T>
    vector<T>& vector<T>::operator=(const vector<T>& rhs) {

        //NOTE: google what this Debug thing is more
        Debug("Assignment operator..." << std::endl);
        if (this == &rhs) {

            return *this;
        }

        delete[] element_array;

        used_slots = rhs.used_slots;
        aloc_slots = rhs.aloc_slots;
        element_array = new value_type[aloc_slots];

        for (size_type i = 0; i < used_slots; ++i) {

            element_array[i] = rhs.element_array[i];
        }

        return *this;
    }

    template <typename T>
    vector<T>& vector<T>::operator=(vector<T>&& rhs) noexcept {

        if (this == &rhs) {

            return *this;
        }

        delete[] element_array;

        element_array = rhs.element_array;
        aloc_slots = rhs.aloc_slots;
        used_slots = rhs.used_slots;

        rhs.element_array = nullptr;
        rhs.aloc_slots = 0;
        rhs.used_slots = 0;

        return *this;
    }

    template <typename T>
    void vector<T>::reserve(size_type new_capacity) {

        Debug("Reserve..." << std::endl);

        if (new_capacity == aloc_slots) {

            return; // Allocated memory is already the right size.
        }

        if (new_capacity < used_slots) {

            new_capacity = used_slots; // Cannot allocate less than currently used.
        }

        value_type* bigger = new value_type[new_capacity];

        for (size_type i = 0; i < used_slots; ++i) {

            bigger[i] = std::move(element_array[i]);
        }

        delete[] element_array;

        aloc_slots = new_capacity;
        element_array = bigger;
    }

    template <typename T>
    bool vector<T>::insert_element(const value_type& entry) {

        Debug("Insert element..." << std::endl);
        size_type pos = search_element(entry);

        // Reject duplicates
        if (pos != NOT_FOUND) {

            return false;
        }

        if (used_slots == aloc_slots) {

            reserve(aloc_slots == 0 ? 1 : aloc_slots * 2);
        }

        element_array[used_slots] = entry;
        ++used_slots;
        return true;
    }

    template <typename T>
    bool vector<T>::erase_element(const value_type& item) {

        size_type index = search_element(item);

        if (index == NOT_FOUND) {

            return false;
        }

        for (size_type i = index; i + 1 < used_slots; ++i) {

            element_array[i] = std::move(element_array[i + 1]);
        }

        --used_slots;
        return true;
    }

    template <typename T>
    void vector<T>::push_back(const value_type& entry) {

        if (used_slots == aloc_slots) {

            reserve(aloc_slots == 0 ? 1 : aloc_slots * 2);
            // rewrite this NOTE
        }

        element_array[used_slots] = entry;
        ++used_slots;
    }

    template <typename T>
    void vector<T>::pop_back() {

        assert(used_slots > 0);
        --used_slots;
    }

    template <typename T>
    void vector<T>::clear() noexcept {

        used_slots = 0;
    }

    template <typename T>
    typename vector<T>::value_type& vector<T>::operator[](size_type index) {

        assert(index < used_slots);
        return element_array[index];
    }

    template <typename T>
    const typename vector<T>::value_type& vector<T>::operator[](size_type index) const {

        assert(index < used_slots);
        return element_array[index];
    }

    template <typename T>
    typename vector<T>::value_type& vector<T>::at(size_type index) {

        if (index >= used_slots) {

            throw std::out_of_range("vector::at: index out of range");
        }
        return element_array[index];
    }

    template <typename T>
    const typename vector<T>::value_type& vector<T>::at(size_type index) const {

        if (index >= used_slots) {

            throw std::out_of_range("vector::at: index out of range");
        }
        return element_array[index];
    }

    template <typename T>
    typename vector<T>::size_type vector<T>::size() const noexcept {

        return used_slots;
    }

    template <typename T>
    typename vector<T>::size_type vector<T>::capacity() const noexcept {

        return aloc_slots;
    }

    template <typename T>
    bool vector<T>::empty() const noexcept {

        return used_slots == 0;
    }

    template <typename T>
    void vector<T>::print_elements() const {

        std::cout << "Element List:" << std::endl;
        for (size_type i = 0; i < used_slots; ++i) {

            std::cout << "- " << element_array[i] << std::endl;
        }
    }

    template <typename T>
    typename vector<T>::value_type* vector<T>::begin() noexcept {

        return element_array;
    }

    template <typename T>
    const typename vector<T>::value_type* vector<T>::begin() const noexcept {

        return element_array;
    }

    template <typename T>
    typename vector<T>::value_type* vector<T>::end() noexcept {

        return element_array + used_slots;
    }

    template <typename T>
    const typename vector<T>::value_type* vector<T>::end() const noexcept {

        return element_array + used_slots;
    }

    template <typename T>
    typename vector<T>::size_type vector<T>::search_element(const value_type& element) const {

        for (size_type i = 0; i < used_slots; ++i) {

            if (element == element_array[i]) {

                return i;
            }
        }

        return NOT_FOUND;
    }
}

#endif // VECTOR_H