enum CacheItemType
{
    RAW,
    VECTOR
};

struct CacheItem
{
    union
    {
        //struct
        //{
        //CacheItemType type;// : 4;
        uint32_t size;// : 28;
        //};
        //uint32_t metadata;
    };
    unsigned char* data = nullptr;

    CacheItem(/*CacheItemType _type, */uint32_t _size, unsigned char* _data)
    {
        //type = _type;
        size = _size;
        data = _data;
    }

    ~CacheItem()
    {
        // Don't delete here. On cache push_back, data is deleted,
        // even if pointer still points to valid memory

        //if (data)
        //    delete data;
    }
};

struct SerializeContext
{
    //std::vector<unsigned char*> data_ptrs;
    uint32_t sum_item_size = 0;
    unsigned char* insert_ptr = nullptr;
    std::fstream* stream = nullptr;
    bool reading = false;

    /*template<typename T>
    std::vector<T>& apply(std::vector<T>& arr,
        typename std::enable_if<std::is_base_of<Serializable, T>::value>::type* = 0)
    {

    }

    template<typename T>
    std::vector<T>& apply(std::vector<T>& arr,
        typename std::enable_if<!std::is_base_of<Serializable, T>::value>::type* = 0)
    {

    }*/

    template<typename T>
    T& apply(T& value)
    {
        if (stream)
        {
            // Read/write
            if (reading)
                stream->read((char*)&value, sizeof(T));
            else
            {
                //memcpy(insert_ptr, &value, sizeof(T));
                //insert_ptr += sizeof(T);

                stream->write((char*)&value, sizeof(T));
            }
        }
        else // Measure item size
            sum_item_size += sizeof(T);

        return value;
    }

    /*template <typename T, std::size_t N>
    T(&apply(T(&value)[N]))[N]
    {

    }

    template<typename T, typename FuncType, typename... Ts>
    T& invoke(FuncType func, Ts&&... args)
    {

    }*/
};

class Serializable
{
    virtual void cache(SerializeContext* cache) = 0;
};

class CacheContext
{
    std::vector<CacheItem> cache;
    CacheItem* cache_lookup;
    uint32_t cache_i;

    std::fstream stream;

    struct Header
    {
        uint32_t item_count = 0;
    } header;

    uint32_t item_count;

public:

    void init(const char* filename)
    {
        stream.open(filename, std::ios::in | std::ios::out | std::ios::binary);

        if (!stream)
        {
            // Create the file
            stream.open(filename, std::ios::out | std::ios::binary);
            stream.close(); // Close after creation

            // Reopen in read-write mode
            stream.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        }

        if (stream)
        {
            stream.read((char*)&header, sizeof(Header));
            if (!stream)
            {
                stream.close(); // Close after creation
                stream.open(filename, std::ios::in | std::ios::out | std::ios::binary);

                stream.write((const char*)&header, sizeof(Header));
            }
            else
            {
                item_count = header.item_count;
            }
        }
        else
        {
            stream.close();

            // Reopen in read-write mode
            stream.open(filename, std::ios::in | std::ios::out | std::ios::binary);
            stream.write((const char*)&header, sizeof(Header));
        }
        if (!stream)
        {
            int a = 5;
        }

        /*if (!cache_stream)
        {
            // Create the file
            cache_stream.open(filename, std::ios::out | std::ios::binary);
            cache_stream.close(); // Close after creation

            // Reopen in read-write mode
            cache_stream.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        }*/

        cache_i = 0;
        if (cache.size())
            cache_lookup = &cache[0];
    }

    ~CacheContext()
    {
        clear();
    }

    void clear()
    {
        for (CacheItem& item : cache)
        {
            if (item.data)
                delete item.data;
        }
        cache.clear();
    }

    void finalize()
    {
        header.item_count = cache_i;

        // Finalize header
        stream.seekp(0, std::ios::beg);
        stream.write((const char*)&header, sizeof(Header));

        stream.close();
    }

    bool missing()
    {
        return (cache_i >= item_count);
    }

    void write_item(uint32_t size, unsigned char* src_data)
    {
        if (!stream)
        {
            int a = 5;
        }

        // if save cache in memory
            //CacheItem cache_item(/*CacheItemType::VECTOR, */size, data);
            //cache.push_back(cache_item);
        // else
            //cache_stream.write(reinterpret_cast<const char*>(&cache_item.metadata), sizeof(cache_item.metadata));
            //cache_stream.write(reinterpret_cast<const char*>(&cache_item.type), sizeof(CacheItemType));
        stream.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
        stream.write(reinterpret_cast<const char*>(src_data), size);

        cache_i++;
    }

    void read_item(int32_t size, unsigned char* dest_data)
    {
        if (!stream)
        {
            int a = 5;
        }

        // if cache in memory
            //CacheItem& cache_item = cache_lookup[cache_i++]
        // else

        stream.read((char*)dest_data, size);

        cache_i++;
    }

    uint32_t read_size()
    {
        uint32_t size;
        stream.read((char*)&size, sizeof(uint32_t));
        return size;
    }

    // Cache vector as one contiguous memory item
    template<typename T>
    std::vector<T>& apply(std::vector<T>& arr,
        typename std::enable_if<std::is_base_of<Serializable, T>::value>::type* = 0)
    {
        if (cache_i < item_count)
        {
            uint32_t size = read_size();

            // Get size of first item
            SerializeContext context;
            arr[0].cache(&context);

            uint32_t arr_length = (size / context.sum_item_size);
            context.stream = &stream;
            context.reading = true;

            for (size_t i = 0; i < arr_length; i++)
                arr[i].cache(&context);

            cache_i++;

            return arr;
        }
        else
        {
            SerializeContext context;

            // Get size of first item
            arr[0].cache(&context);

            size_t count = arr.size();
            uint32_t total_size = context.sum_item_size * count;

            stream.write(reinterpret_cast<const char*>(&total_size), sizeof(uint32_t));
            context.stream = &stream;

            // Write contiguous data blob
            for (size_t i = 0; i < count; i++)
                arr[i].cache(&context);

            /*unsigned char* data = new unsigned char[total_size];
            context.insert_ptr = data;

            for (size_t i = 0; i < count; i++)
                arr[i].cache(&context);

            write_item(total_size, data);
            delete[] data;*/

            cache_i++;


            return arr;
        }
    }

    template<typename T>
    std::vector<T>& apply(std::vector<T>& arr,
        typename std::enable_if<!std::is_base_of<Serializable, T>::value>::type* = 0)
    {
        if (cache_i < item_count)
        {
            uint32_t size = read_size();
            arr.resize(size / sizeof(T));

            read_item(size, (unsigned char*)arr.data());

            return arr;
        }
        else
        {
            uint32_t size = arr.size() * sizeof(T);
            unsigned char* data = new unsigned char[size];
            memcpy(data, &arr[0], size);

            write_item(size, data);
            return arr;
        }
    }

    template<typename T>
    T& apply(T& value)
    {
        if (cache_i < item_count)
        {
            uint32_t size = read_size();
            read_item(size, (unsigned char*)&value);
            return value;
        }
        else
        {
            // Allocate memory to store the value.
            uint32_t size = sizeof(T);
            unsigned char* data = new unsigned char[size];
            new (data) T(value);

            write_item(size, data);
            return value;
        }
    }

    template <typename T, std::size_t N>
    T(&apply(T(&value)[N]))[N]
        {
            if (cache_i < item_count)
            {
                uint32_t size = read_size();
                read_item(size, (unsigned char*)&value);
                return value;
            }
            else
            {
                // Allocate memory to store the value.
                uint32_t size = sizeof(value);
                unsigned char* data = new unsigned char[size];
                memcpy(data, &value, size);

                // Store the cached item.
                write_item(size, data);
                return value;
            }
        }

        template<typename T, typename FuncType, typename... Ts>
    T& invoke(FuncType func, Ts&&... args)
    {
        if (cache_i < item_count)
        {
            T value;
            uint32_t size = read_size();
            read_item(size, (unsigned char*)&value);
            return value;
        }
        else
        {
            // Call the function to compute the value.
            T value = func(std::forward<Ts>(args)...);

            uint32_t size = sizeof(T);
            unsigned char* data = new unsigned char[size];
            new (data) T(value);

            write_item(size, data);
            return value;
        }
    }
};
