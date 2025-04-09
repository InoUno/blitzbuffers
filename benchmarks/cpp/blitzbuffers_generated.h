
/**
 * This file has been auto-generated via blitzbuffers. Do not edit it directly.
 */

/**
 * This file has been auto-generated via blitzbuffers. Do not edit it directly.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <optional>
#include <variant>
#include <vector>

#ifdef __cpp_lib_span
#include <span>
#endif

namespace blitzbuffers
{
    // https://en.cppreference.com/w/cpp/utility/variant/visit
    template <class... Ts>
    struct overloaded : Ts...
    {
        using Ts::operator()...;
    };

    // explicit deduction guide (not needed as of C++20)
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    // https://stackoverflow.com/questions/64017982/c-equivalent-of-rust-enums
    template <typename Val, typename... Ts>
    inline auto match(Val&& val, Ts... ts)
    {
        return std::visit(overloaded { ts... }, val);
    }

    using offset_t = uint32_t;

    template <typename E>
    constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept
    {
        return static_cast<typename std::underlying_type<E>::type>(e);
    }

    class FixedSizeBufferBackend
    {
    private:
        uint8_t* builder_buffer;
        offset_t current_size = 0;

    public:
        FixedSizeBufferBackend(const FixedSizeBufferBackend&) = delete;            // No copying allowed
        FixedSizeBufferBackend& operator=(const FixedSizeBufferBackend&) = delete; // No copy assignment
		FixedSizeBufferBackend(FixedSizeBufferBackend&& other)
			: builder_buffer(other.builder_buffer)
		{
			other.builder_buffer = nullptr;
		}

        FixedSizeBufferBackend(uint8_t* buffer)
        : builder_buffer(buffer)
        {
        }

        FixedSizeBufferBackend(offset_t max_size)
        : FixedSizeBufferBackend(new uint8_t[max_size] { 0 })
        {
        }

        ~FixedSizeBufferBackend()
        {
            delete[] builder_buffer;
        }

        std::pair<offset_t, uint8_t*> add_buffer(offset_t size)
        {
            auto buffer = builder_buffer + current_size;
            auto offset = current_size;
            current_size += size;

            return { offset, buffer };
        }

        uint8_t* get_new_buffer(offset_t size)
        {
            auto buffer = builder_buffer + current_size;
            current_size += size;
            return buffer;
        }

        offset_t add_string(const char* value, size_t len)
        {
            offset_t size   = static_cast<offset_t>(len + 1);
            uint8_t* buffer = builder_buffer + current_size;

            memcpy(buffer, value, len);
            buffer[size] = 0;

            auto offset = current_size;
            current_size += size;
            return offset;
        }

        offset_t get_size()
        {
            return current_size;
        }

        void clear()
        {
            memset(builder_buffer, 0, current_size);
            current_size = 0;
        }

        std::pair<offset_t, uint8_t*> build()
        {
            return { current_size, builder_buffer };
        }

        std::vector<uint8_t> build_vec()
        {
            return std::vector<uint8_t>(builder_buffer, builder_buffer + current_size);
        }
    };

    struct BufferTracker
    {
        uint8_t* buffer;
        offset_t size;
        offset_t free;

        inline offset_t used()
        {
            return size - free;
        }
    };

    class ExpandableBufferBackend
    {
    private:
        BufferTracker current_tracker;
        std::vector<BufferTracker> previous_trackers = {};
        offset_t current_size                        = 0;
        uint8_t* final_buffer                        = nullptr;

        const offset_t default_buffer_size;

    public:
        ExpandableBufferBackend(const ExpandableBufferBackend&) = delete;            // No copying allowed
        ExpandableBufferBackend& operator=(const ExpandableBufferBackend&) = delete; // No copy assignment
        ExpandableBufferBackend(ExpandableBufferBackend&& other)
            : current_tracker(std::move(other.current_tracker))
            , previous_trackers(std::move(other.previous_trackers))
            , current_size(other.current_size)
            , final_buffer(other.final_buffer)
            , default_buffer_size(other.default_buffer_size)
        {
            other.final_buffer = nullptr;
            other.current_tracker.buffer = nullptr;
            other.previous_trackers.clear();
        }

        ExpandableBufferBackend(offset_t default_buffer_size_ = 256)
        : default_buffer_size(default_buffer_size_)
        , current_tracker({ new uint8_t[default_buffer_size_] { 0 }, default_buffer_size_, default_buffer_size_ })
        {
        }

        ~ExpandableBufferBackend()
        {
            for (auto& buffer_tracker : previous_trackers)
            {
                delete[] buffer_tracker.buffer;
            }
            previous_trackers.clear();

            delete[] current_tracker.buffer;
            current_tracker.buffer = nullptr;
            delete final_buffer;
        }

        std::pair<offset_t, uint8_t*> add_buffer(offset_t size)
        {
            ensure_capacity(size);

            uint8_t* buffer = current_tracker.buffer + current_tracker.used();
            current_tracker.free -= size;

            auto offset = current_size;
            current_size += size;

            return { offset, buffer };
        }

        uint8_t* get_new_buffer(offset_t size)
        {
            ensure_capacity(size);

            uint8_t* buffer = current_tracker.buffer + current_tracker.used();
            current_tracker.free -= size;

            current_size += size;
            return buffer;
        }

        offset_t add_string(const char* value, size_t len)
        {
            offset_t size = static_cast<offset_t>(len + 1);

            ensure_capacity(size);

            uint8_t* buffer = current_tracker.buffer + current_tracker.used();
            current_tracker.free -= size;

            memcpy(buffer, value, len);

            auto offset = current_size;
            current_size += size;
            return offset;
        }

        offset_t get_size()
        {
            return current_size;
        }

        void ensure_capacity(size_t size)
        {
            if (current_tracker.free >= size)
            {
                return;
            }

            offset_t size_to_use = static_cast<offset_t>(size > default_buffer_size ? size : default_buffer_size);

            previous_trackers.push_back(current_tracker);
            current_tracker = {
                new uint8_t[size_to_use] { 0 },
                size_to_use,
                size_to_use
            };
        }

        void clear()
        {
            delete[] final_buffer;
            final_buffer = nullptr;

            delete[] current_tracker.buffer;
            current_tracker = { new uint8_t[default_buffer_size] { 0 }, default_buffer_size, default_buffer_size };

            for (auto& buffer_tracker : previous_trackers)
            {
                delete[] buffer_tracker.buffer;
            }
            previous_trackers.clear();

            current_size = 0;
        }

        std::pair<offset_t, uint8_t*> build()
        {
            if (previous_trackers.size() == 0)
            {
                return { current_size, current_tracker.buffer };
            }

            delete final_buffer;
            final_buffer = new uint8_t[current_size];
            auto offset  = 0;
            for (auto& buffer_tracker : previous_trackers)
            {
                memcpy(final_buffer + offset, buffer_tracker.buffer, buffer_tracker.used());
                offset += buffer_tracker.used();
            }
            memcpy(final_buffer + offset, current_tracker.buffer, current_tracker.used());
            offset += current_tracker.used();

            return { current_size, final_buffer };
        }

        std::vector<uint8_t> build_vec()
        {
            if (previous_trackers.size() == 0)
            {
                return std::vector<uint8_t>(current_tracker.buffer, current_tracker.buffer + current_size);
            }

            std::vector<uint8_t> out;
            out.resize(current_size);
            auto offset = 0;
            for (auto& buffer_tracker : previous_trackers)
            {
                memcpy(out.data() + offset, buffer_tracker.buffer, buffer_tracker.used());
                offset += buffer_tracker.used();
            }
            memcpy(out.data() + offset, current_tracker.buffer, current_tracker.used());

            return out;
        }
    };

    template <class BufferBackend>
    class BufferWriterBase
    {
    protected:
        BufferBackend& __backend;
        uint8_t* __buffer;
        const offset_t __self_offset;

        BufferWriterBase(BufferBackend& backend, uint8_t* buffer, offset_t self_offset)
        : __backend(backend)
        , __buffer(buffer)
        , __self_offset(self_offset)
        {
        }
    };

    template <typename T, typename BufferType>
    class PrimitiveContainer
    {
    private:
        BufferType __buffer;

    public:
        inline PrimitiveContainer(BufferType buffer)
        : __buffer(buffer)
        {
        }

        template <typename Backend>
        inline PrimitiveContainer(Backend&, uint8_t* buffer, offset_t)
        : __buffer(buffer)
        {
        }

        inline operator T() const
        {
            return value();
        }

        inline T value() const
        {
            T value;
            std::memcpy(&value, __buffer, sizeof(T));
            return value;
        }

        template <typename U = BufferType>
        inline typename std::enable_if<!std::is_const<U>::value, PrimitiveContainer&>::type operator=(const T& value)
        {
            set(value);
            return *this;
        }

        template <typename U = BufferType>
        inline typename std::enable_if<!std::is_const<U>::value, void>::type set(const T& value)
        {
            std::memcpy(__buffer, &value, sizeof(T));
        }

        constexpr static offset_t blitz_size()
        {
            return sizeof(T);
        }

        inline static bool check(const uint8_t* buffer, const offset_t length)
        {
            return length >= sizeof(T);
        }

        template <typename T>
        friend std::ostream& operator<<(std::ostream& os, const PrimitiveContainer<T, const uint8_t*>& container);
    };

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const PrimitiveContainer<T, const uint8_t*>& container)
    {
        if constexpr (std::is_same_v<T, char>)
        {
            os << (int)(container.value());
        }
        else
        {
            os << container.value();
        }
        return os;
    }

    template <typename T>
    struct remove_primitive_container
    {
        using type = T;
    };

    template <typename T, typename U>
    struct remove_primitive_container<PrimitiveContainer<T, U>>
    {
        using type = T;
    };

    template <class T>
    using remove_primitive_container_t = typename remove_primitive_container<T>::type;

    template <typename T, typename BufferType>
    inline constexpr PrimitiveContainer<T, BufferType> make_primitive(BufferType buffer, offset_t offset)
    {
        return PrimitiveContainer<T, BufferType>(buffer + offset);
    }

    inline static bool check_string(const uint8_t* buffer, const offset_t length)
    {
        if (length < sizeof(offset_t))
        {
            return false;
        }

        offset_t offset = PrimitiveContainer<offset_t, const uint8_t*>(buffer);
        if (offset == 0)
        {
            return true;
        }
        if (offset >= length)
        {
            return false;
        }

        do
        {
            if (buffer[offset] == '\0')
            {
                return true;
            }
            offset++;
        } while (offset < length);

        return false;
    }

    template <typename T>
    inline static bool check_vector(const uint8_t* buffer, const offset_t length)
    {
        if (length < sizeof(offset_t))
        {
            return false;
        }

        offset_t offset = PrimitiveContainer<offset_t, const uint8_t*>(buffer);
        if (offset == 0)
        {
            return true;
        }
        if (offset >= length)
        {
            return false;
        }

        const offset_t vector_length = PrimitiveContainer<offset_t, const uint8_t*>(buffer + offset);
        offset += sizeof(offset_t);

        constexpr offset_t entry_size = T::blitz_size();
        const offset_t end_of_vector  = offset + vector_length * entry_size;
        if (end_of_vector > length)
        {
            return false;
        }

        while (offset < end_of_vector)
        {
            T::check(buffer + offset, length - offset);
            offset += entry_size;
        }

        return true;
    }

    template <typename T>
    class StringPointer
    {
    private:
        const uint8_t* __buffer;
        PrimitiveContainer<offset_t, const uint8_t*> __offset;

    public:
        inline StringPointer(const uint8_t* buffer)
        : __buffer(buffer)
        , __offset(buffer)
        {
        }

        inline operator T() const
        {
            return value();
        }

        inline T value() const
        {
            return (T)(__buffer + static_cast<offset_t>(__offset));
        }

        constexpr static offset_t blitz_size()
        {
            return sizeof(offset_t);
        }

        inline static bool check(const uint8_t* buffer, const offset_t length)
        {
            return check_string(buffer, length);
        }

        friend bool operator==(const StringPointer<const char*>& lhs, const StringPointer<const char*>& rhs);

        friend std::ostream& operator<<(std::ostream& os, const StringPointer<const char*>& ptr)
        {
            os << "\"" << ptr.value() << "\"";
            return os;
        }
    };

    inline bool operator==(const StringPointer<const char*>& lhs, const StringPointer<const char*>& rhs)
    {
        return strcmp(lhs.value(), rhs.value()) == 0;
    }

    template <class BufferBackend>
    class StringWriter
    {
    private:
        BufferBackend& __backend;
        uint8_t* __buffer;
        PrimitiveContainer<offset_t, uint8_t*> __offset;
        const offset_t __self_offset;

    public:
        StringWriter(BufferBackend& backend, uint8_t* buffer, offset_t self_offset)
        : __backend(backend)
        , __buffer(buffer)
        , __offset(buffer)
        , __self_offset(self_offset)
        {
        }

        constexpr static offset_t blitz_size()
        {
            return sizeof(offset_t);
        }

#ifdef __cpp_lib_string_view
        void insert_string(std::string_view value)
        {
            this->__offset = this->__backend.add_string(value.data(), value.length()) - this->__self_offset;
        }

        StringWriter& operator=(std::string_view value)
        {
            this->insert_string(value);
            return *this;
        }
#else
        void insert_string(const char* value)
        {
            this->__offset = this->__backend.add_string(value, strlen(value)) - this->__self_offset;
        }

        StringWriter& operator=(const char* value)
        {
            this->insert_string(value);
            return *this;
        }

        StringWriter& operator=(std::string value)
        {
            this->insert_string(value.c_str());
            return *this;
        }
#endif
    };

    template <class T, class BufferBackend>
    class VectorWriter
    {
    private:
        BufferBackend& __backend;
        uint8_t* __buffer;
        const offset_t __start_offset;
        const offset_t __element_size;

    public:
        VectorWriter(BufferBackend& backend, uint8_t* buffer, offset_t start_offset)
        : __backend(backend)
        , __buffer(buffer)
        , __start_offset(start_offset)
        , __element_size(T::blitz_size())
        {
        }

        static VectorWriter make_and_set_offset(BufferBackend& backend, offset_t length, offset_t buffer_offset, PrimitiveContainer<offset_t, uint8_t*> out_offset)
        {
            auto [offset, buffer] = backend.add_buffer(sizeof(offset_t) + T::blitz_size() * length);

            PrimitiveContainer<offset_t, uint8_t*> _length = { buffer };
            _length                                        = length;

            out_offset = offset - buffer_offset;
            return VectorWriter<T, BufferBackend>(backend, buffer + sizeof(offset_t), offset + sizeof(offset_t));
        }

        T operator[](int index)
        {
            auto element_offset = __element_size * index;
            return T(__backend, __buffer + element_offset, __start_offset + element_offset);
        }
    };
    template <class T, class BufferBackend>
    class VectorWriterPointer
    {
    private:
        BufferBackend& __backend;
        PrimitiveContainer<offset_t, uint8_t*> __offset;
        const offset_t __self_offset;

    public:
        VectorWriterPointer(BufferBackend& backend, PrimitiveContainer<offset_t, uint8_t*> offset, offset_t self_offset)
        : __backend(backend)
        , __offset(offset)
        , __self_offset(self_offset)
        {
        }

        constexpr static offset_t blitz_size()
        {
            return sizeof(offset_t);
        }

#ifdef __cpp_lib_span
        template <typename U>
        void insert(std::span<U> _raw_span)
        {
            auto length = static_cast<offset_t>(_raw_span.size());
            auto writer = VectorWriter<T, BufferBackend>::make_and_set_offset(this->__backend, length, this->__self_offset, this->__offset);
            for (offset_t i = 0; i < length; i++)
            {
                writer[i] = _raw_span[i];
            }
        }
#endif // __cpp_lib_span

        template <typename U>
        void insert(std::initializer_list<U> _raw_init)
        {
            auto length = static_cast<offset_t>(_raw_init.size());
            auto writer = VectorWriter<T, BufferBackend>::make_and_set_offset(this->__backend, length, this->__self_offset, this->__offset);
            auto iter   = _raw_init.begin();
            for (offset_t i = 0; i < length; i++)
            {
                writer[i] = *iter;
                iter++;
            }
        }

        template <typename U>
        void insert(const std::vector<U>& _raw_vec)
        {
            auto length = static_cast<offset_t>(_raw_vec.size());
            auto writer = VectorWriter<T, BufferBackend>::make_and_set_offset(this->__backend, length, this->__self_offset, this->__offset);
            for (offset_t i = 0; i < length; i++)
            {
                writer[i] = _raw_vec[i];
            }
        }

        template <typename U>
        void insert(const U* data, const offset_t data_length)
        {
            auto writer = VectorWriter<T, BufferBackend>::make_and_set_offset(this->__backend, data_length, this->__self_offset, this->__offset);
            for (offset_t i = 0; i < data_length; i++)
            {
                writer[i] = data[i];
            }
        }

        template <typename U>
        VectorWriterPointer<T, BufferBackend>& operator=(const std::vector<U>& _raw_vec)
        {
            this->insert(_raw_vec);
            return *this;
        }
    };
    template <typename T>
    class VectorIter
    {
    private:
        const uint8_t* buffer;
        const PrimitiveContainer<offset_t, const uint8_t*> len;
        const offset_t entry_size;
        offset_t index = 0;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;

        inline VectorIter(const uint8_t* _buffer, offset_t _entry_size, offset_t _start_index = 0)
        : len(_buffer)
        , buffer(_buffer + sizeof(offset_t))
        , entry_size(_entry_size)
        , index(_start_index)
        {
        }

        inline VectorIter<T> operator++()
        {
            index += entry_size;
            return *this;
        }

        inline T operator*()
        {
            return T(buffer + index);
        }

        friend bool operator==(const VectorIter<T>& a, const VectorIter<T>& b)
        {
            return a.buffer == b.buffer && a.index == b.index;
        }

        friend bool operator!=(const VectorIter<T>& a, const VectorIter<T>& b)
        {
            return a.buffer != b.buffer || a.index != b.index;
        }
    };

    template <typename T>
    class Vector
    {
    private:
        const uint8_t* buffer;

    public:
        inline Vector(const uint8_t* _buffer)
        : buffer(_buffer)
        {
        }

        inline constexpr static offset_t blitz_size()
        {
            return sizeof(offset_t);
        }

        inline offset_t offset() const
        {
            const PrimitiveContainer<offset_t, const uint8_t*> offset(buffer);
            return offset.value();
        }

        inline offset_t length() const
        {
            const PrimitiveContainer<offset_t, const uint8_t*> len(buffer + offset());
            return len.value();
        }

        inline const remove_primitive_container_t<T>* data_ptr() const
        {
            return static_cast<const remove_primitive_container_t<T>*>(buffer + offset() + sizeof(offset_t));
        }

        inline VectorIter<T> begin() const
        {
            return VectorIter<T>(buffer + offset(), T::blitz_size());
        }

        inline VectorIter<T> end() const
        {
            return VectorIter<T>(buffer + offset(), T::blitz_size(), length() * T::blitz_size());
        }

        inline T operator[](int index) const
        {
            return T(buffer + offset() + sizeof(offset_t) + index * T::blitz_size());
        }

        inline static bool check(const uint8_t* buffer, const offset_t length)
        {
            return check_vector<T>(buffer, length);
        }

        template <typename T>
        friend bool operator==(const Vector<T>& lhs, const Vector<T>& rhs);

        template <typename T>
        friend std::ostream& operator<<(std::ostream& os, const Vector<T>& vector);
    };

    template <typename T>
    inline bool operator==(const Vector<T>& lhs, const Vector<T>& rhs)
    {
        if (lhs.length() != rhs.length())
        {
            return false;
        }
        for (offset_t i = 0; i < lhs.length(); i++)
        {
            if (lhs[i] != rhs[i])
            {
                return false;
            }
        }
        return true;
    }

    template <typename T>
    inline std::ostream& operator<<(std::ostream& os, const Vector<T>& vector)
    {
        os << "Vector[";
        if (vector.length() > 0)
        {
            os << vector[0];
            for (offset_t i = 1; i < vector.length(); i++)
            {
                if constexpr (std::is_same_v<T, char>)
                {
                    os << ", " << (int)(vector[i]);
                }
                else
                {
                    os << ", " << vector[i];
                }
            }
        }
        os << "]";
        return os;
    }
}

namespace bzb = blitzbuffers;


//
// Forward declarations
//
namespace bench_blitzbuffers
{
    enum class EntityType : uint8_t;
}
// Struct forward declaration Position
namespace bench_blitzbuffers::Position
{
    class Raw;
    class Viewer;

    template <class>
    class Builder;
}
// Struct forward declaration Entity
namespace bench_blitzbuffers::Entity
{
    class Raw;
    class Viewer;

    template <class>
    class Builder;
}

//
// Declarations
//
namespace bench_blitzbuffers
{
    enum class EntityType : uint8_t
    {
        Player = 0,
        Enemy = 1,
        Neutral = 2,
    };

    inline std::ostream& operator<<(std::ostream& os, const EntityType& value)
    {
        switch (value)
        {
        case EntityType::Player:
            os << "Player";
            break;
        case EntityType::Enemy:
            os << "Enemy";
            break;
        case EntityType::Neutral:
            os << "Neutral";
            break;
        default:
            os << "Unknown(" << static_cast<uint8_t>(value) << ")";
            break;
        }
        return os;
    }

}
// Struct declaration Position
namespace bench_blitzbuffers::Position
{
    constexpr static bzb::offset_t blitz_size()
    {
        return 12;
    }

    class Raw {
    public:
        float x;
        float y;
        float z;
    };

    class Viewer
    {
    private:
        const uint8_t* __buffer;

        bzb::PrimitiveContainer<float, const uint8_t*> _get_container_x() const
        {
            return bzb::PrimitiveContainer<float, const uint8_t*>(this->__buffer + 0);
        }

        bzb::PrimitiveContainer<float, const uint8_t*> _get_container_y() const
        {
            return bzb::PrimitiveContainer<float, const uint8_t*>(this->__buffer + 4);
        }

        bzb::PrimitiveContainer<float, const uint8_t*> _get_container_z() const
        {
            return bzb::PrimitiveContainer<float, const uint8_t*>(this->__buffer + 8);
        }


    public:
        constexpr static bzb::offset_t blitz_size()
        {
            return 12;
        }

        Viewer(const uint8_t* buffer);

        float get_x() const
        {
            return this->_get_container_x().value();
        }

        float get_y() const
        {
            return this->_get_container_y().value();
        }

        float get_z() const
        {
            return this->_get_container_z().value();
        }


        static bool check(const uint8_t* buffer, const bzb::offset_t length)
        {
            return length >= 12;
        }

        friend bool operator==(const Viewer& lhs, const Viewer& rhs);

        friend std::ostream& operator<<(std::ostream& os, const Viewer& value);

    };
    template <class BufferBackend>
    class Builder : bzb::BufferWriterBase<BufferBackend>
    {
    private:

    public:
        constexpr static bzb::offset_t blitz_size()
        {
            return 12;
        }

        bzb::PrimitiveContainer<float, uint8_t*> x()
        {
            return { this->__buffer + 0 };
        }

        void set_x(float value)
        {
            this->x() = value;
        }

        bzb::PrimitiveContainer<float, uint8_t*> y()
        {
            return { this->__buffer + 4 };
        }

        void set_y(float value)
        {
            this->y() = value;
        }

        bzb::PrimitiveContainer<float, uint8_t*> z()
        {
            return { this->__buffer + 8 };
        }

        void set_z(float value)
        {
            this->z() = value;
        }

        Builder<BufferBackend>& operator=(Raw _raw);

        Builder(BufferBackend& _backend, uint8_t* _buffer, bzb::offset_t _self_offset);

    };
    template <class BufferBackend>
    static Builder<BufferBackend> new_on(BufferBackend& backend)
    {
        auto offset = backend.get_size();
        auto buffer = backend.get_new_buffer(::bench_blitzbuffers::Position::blitz_size());
        return Builder(backend, buffer, offset);
    }

    static bool check(const uint8_t* buffer, const bzb::offset_t length)
    {
        return ::bench_blitzbuffers::Position::Viewer::check(buffer, length);
    }

    static std::optional<Viewer> view(const uint8_t* buffer, const bzb::offset_t length)
    {
        if (!::bench_blitzbuffers::Position::check(buffer, length)) {
            return std::nullopt;
        }
        return { Viewer(buffer) };
    }

    static Viewer view_unchecked(const uint8_t* buffer)
    {
        return Viewer(buffer);
    }

    static ::bench_blitzbuffers::Position::Raw raw()
    {
        return ::bench_blitzbuffers::Position::Raw {};
    }

    static ::bench_blitzbuffers::Position::Raw raw(::bench_blitzbuffers::Position::Raw _raw)
    {
        return _raw;
    }
}
// Struct declaration Entity
namespace bench_blitzbuffers::Entity
{
    constexpr static bzb::offset_t blitz_size()
    {
        return 21;
    }

    class Raw {
    public:
        ::bench_blitzbuffers::EntityType type;
        ::bench_blitzbuffers::Position::Raw position;
        std::string name;
        std::vector<::bench_blitzbuffers::Entity::Raw> related;
    };

    class Viewer
    {
    private:
        const uint8_t* __buffer;

        bzb::PrimitiveContainer<::bench_blitzbuffers::EntityType, const uint8_t*> _get_container_type() const
        {
            return bzb::PrimitiveContainer<::bench_blitzbuffers::EntityType, const uint8_t*>(this->__buffer + 0);
        }

        bzb::PrimitiveContainer<bzb::offset_t, const uint8_t*> _get_offset_name() const
        {
            return { this->__buffer + 13 };
        }

        bzb::PrimitiveContainer<bzb::offset_t, const uint8_t*> _get_offset_related() const
        {
            return { this->__buffer + 17 };
        }


    public:
        constexpr static bzb::offset_t blitz_size()
        {
            return 21;
        }

        Viewer(const uint8_t* buffer);

        EntityType get_type() const
        {
            return this->_get_container_type().value();
        }

        ::bench_blitzbuffers::Position::Viewer get_position() const
        {
            return ::bench_blitzbuffers::Position::Viewer(this->__buffer + 1);
        }

        const char* get_name() const
        {
            if (this->_get_offset_name() == 0)
            {
                return "";
            }
            return (const char*)(__buffer + this->_get_offset_name() + 13);
        }

        bzb::Vector<::bench_blitzbuffers::Entity::Viewer> get_related() const
        {
            return bzb::Vector<::bench_blitzbuffers::Entity::Viewer>(this->__buffer + 17);
        }


        static bool check(const uint8_t* buffer, const bzb::offset_t length)
        {
            return length >= 21
                && ::bench_blitzbuffers::Position::check(buffer + 1, length - 1)
                && bzb::check_string(buffer + 13, length - 13)
                && bzb::check_vector<::bench_blitzbuffers::Entity::Viewer>(buffer + 17, length - 17);
        }

        friend bool operator==(const Viewer& lhs, const Viewer& rhs);

        friend std::ostream& operator<<(std::ostream& os, const Viewer& value);

    };
    template <class BufferBackend>
    class Builder : bzb::BufferWriterBase<BufferBackend>
    {
    private:
        bzb::PrimitiveContainer<bzb::offset_t, uint8_t*> _offset_name()
        {
            return { this->__buffer + 13 };
        }

        bzb::PrimitiveContainer<bzb::offset_t, uint8_t*> _offset_related()
        {
            return { this->__buffer + 17 };
        }


    public:
        constexpr static bzb::offset_t blitz_size()
        {
            return 21;
        }

        bzb::PrimitiveContainer<::bench_blitzbuffers::EntityType, uint8_t*> type()
        {
            return { this->__buffer + 0 };
        }

        void set_type(::bench_blitzbuffers::EntityType value)
        {
            this->type() = value;
        }

        ::bench_blitzbuffers::Position::Builder<BufferBackend> position()
        {
            return { this->__backend, this->__buffer + 1, this->__self_offset + 1 };
        }

#ifdef __cpp_lib_string_view

        void insert_name(std::string_view value)
        {
            this->_offset_name() = this->__backend.add_string(value.data(), value.length()) - this->__self_offset - 13;
        }

#else // __cpp_lib_string_view

        void insert_name(const std::string& value)
        {
            this->_offset_name() = this->__backend.add_string(value.c_str(), value.size()) - this->__self_offset - 13;
        }

        void insert_name(const char* value)
        {
            this->_offset_name() = this->__backend.add_string(value, strlen(value)) - this->__self_offset - 13;
        }

#endif // __cpp_lib_string_view

        bzb::VectorWriter<::bench_blitzbuffers::Entity::Builder<BufferBackend>, BufferBackend> insert_related(bzb::offset_t _size)
        {
            return bzb::VectorWriter<::bench_blitzbuffers::Entity::Builder<BufferBackend>, BufferBackend>::make_and_set_offset(this->__backend, _size, this->__self_offset + 17, this->_offset_related());
        }

#ifdef __cpp_lib_span

        void insert_related(std::span<::bench_blitzbuffers::Entity::Raw> data)
        {
            auto vector_ptr = bzb::VectorWriterPointer<::bench_blitzbuffers::Entity::Builder<BufferBackend>, BufferBackend>(this->__backend, this->_offset_related(), this->__self_offset + 17);
            vector_ptr.insert(data);
        }

#endif // __cpp_lib_span

        void insert_related(const std::vector<::bench_blitzbuffers::Entity::Raw>& _raw_vec)
        {
            auto vector_ptr = bzb::VectorWriterPointer<::bench_blitzbuffers::Entity::Builder<BufferBackend>, BufferBackend>(this->__backend, this->_offset_related(), this->__self_offset + 17);
            vector_ptr.insert(_raw_vec);
        }

        void insert_related(std::initializer_list<::bench_blitzbuffers::Entity::Raw> data)
        {
            auto vector_ptr = bzb::VectorWriterPointer<::bench_blitzbuffers::Entity::Builder<BufferBackend>, BufferBackend>(this->__backend, this->_offset_related(), this->__self_offset + 17);
            vector_ptr.insert(data);
        }

        void insert_related(const ::bench_blitzbuffers::Entity::Raw* data, const bzb::offset_t data_length)
        {
            auto vector_ptr = bzb::VectorWriterPointer<::bench_blitzbuffers::Entity::Builder<BufferBackend>, BufferBackend>(this->__backend, this->_offset_related(), this->__self_offset + 17);
            vector_ptr.insert(data, data_length);
        }

        Builder<BufferBackend>& operator=(Raw _raw);

        Builder(BufferBackend& _backend, uint8_t* _buffer, bzb::offset_t _self_offset);

    };
    template <class BufferBackend>
    static Builder<BufferBackend> new_on(BufferBackend& backend)
    {
        auto offset = backend.get_size();
        auto buffer = backend.get_new_buffer(::bench_blitzbuffers::Entity::blitz_size());
        return Builder(backend, buffer, offset);
    }

    static bool check(const uint8_t* buffer, const bzb::offset_t length)
    {
        return ::bench_blitzbuffers::Entity::Viewer::check(buffer, length);
    }

    static std::optional<Viewer> view(const uint8_t* buffer, const bzb::offset_t length)
    {
        if (!::bench_blitzbuffers::Entity::check(buffer, length)) {
            return std::nullopt;
        }
        return { Viewer(buffer) };
    }

    static Viewer view_unchecked(const uint8_t* buffer)
    {
        return Viewer(buffer);
    }

    static ::bench_blitzbuffers::Entity::Raw raw()
    {
        return ::bench_blitzbuffers::Entity::Raw {};
    }

    static ::bench_blitzbuffers::Entity::Raw raw(::bench_blitzbuffers::Entity::Raw _raw)
    {
        return _raw;
    }
}

//
// Definitions
//
// Struct definition Position
namespace bench_blitzbuffers::Position
{
    inline Viewer::Viewer(const uint8_t* _buffer)
    : __buffer(_buffer)
    {}

    inline bool operator==(const Viewer& lhs, const Viewer& rhs)
    {
        return true
            && lhs.get_x() == rhs.get_x()
            && lhs.get_y() == rhs.get_y()
            && lhs.get_z() == rhs.get_z()
        ;
    }

    inline std::ostream& operator<<(std::ostream& os, const Viewer& value)
    {
        os << "Position{";
        os << "x=" << value.get_x() << "; ";
        os << "y=" << value.get_y() << "; ";
        os << "z=" << value.get_z() << "; ";
        os << "}";
        return os;
    }

    template <class BufferBackend>
    inline Builder<BufferBackend>::Builder(BufferBackend& _backend, uint8_t* _buffer, bzb::offset_t _self_offset)
    : bzb::BufferWriterBase<BufferBackend>(_backend, _buffer, _self_offset)
    {}

    template <class BufferBackend>
    inline Builder<BufferBackend>& Builder<BufferBackend>::operator=(Raw _raw)
    {
        this->x() = _raw.x;
        this->y() = _raw.y;
        this->z() = _raw.z;
        return *this;
    }
}
// Struct definition Entity
namespace bench_blitzbuffers::Entity
{
    inline Viewer::Viewer(const uint8_t* _buffer)
    : __buffer(_buffer)
    {}

    inline bool operator==(const Viewer& lhs, const Viewer& rhs)
    {
        return true
            && lhs.get_type() == rhs.get_type()
            && lhs.get_position() == rhs.get_position()
            && lhs.get_name() == rhs.get_name()
            && lhs.get_related() == rhs.get_related()
        ;
    }

    inline std::ostream& operator<<(std::ostream& os, const Viewer& value)
    {
        os << "Entity{";
        os << "type=" << value.get_type() << "; ";
        os << "position=" << value.get_position() << "; ";
        os << "name=\"" << value.get_name() << "\"; ";
        os << "related=" << value.get_related() << "; ";
        os << "}";
        return os;
    }

    template <class BufferBackend>
    inline Builder<BufferBackend>::Builder(BufferBackend& _backend, uint8_t* _buffer, bzb::offset_t _self_offset)
    : bzb::BufferWriterBase<BufferBackend>(_backend, _buffer, _self_offset)
    {}

    template <class BufferBackend>
    inline Builder<BufferBackend>& Builder<BufferBackend>::operator=(Raw _raw)
    {
        this->type() = _raw.type;
        this->position() = _raw.position;
        this->insert_name(_raw.name);
        this->insert_related(_raw.related);
        return *this;
    }
}
