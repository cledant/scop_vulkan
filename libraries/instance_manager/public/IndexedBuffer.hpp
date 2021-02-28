#ifndef SCOP_VULKAN_INDEXEDBUFFER_HPP
#define SCOP_VULKAN_INDEXEDBUFFER_HPP

template<typename InstanceType>
using UpdaterFct =
  std::function<void(uint32_t bufferIndex, InstanceType const &info)>;

template<typename InstanceType>
class IndexedBuffer
{
  public:
    IndexedBuffer() = default;
    ~IndexedBuffer() = default;
    IndexedBuffer(IndexedBuffer const &src) = delete;
    IndexedBuffer &operator=(IndexedBuffer const &rhs) = delete;
    IndexedBuffer(IndexedBuffer &&src) = delete;
    IndexedBuffer &operator=(IndexedBuffer &&rhs) = delete;

    void setMaxInstanceNb(uint32_t maxInstanceNb);
    uint32_t getCurrentInstanceNb() const;
    uint32_t getMaxInstanceNb() const;

    uint32_t addInstance(InstanceType const &info,
                         UpdaterFct<InstanceType> update = nullptr);
    bool removeInstance(uint32_t instanceIndex,
                        UpdaterFct<InstanceType> update = nullptr);
    bool updateInstance(uint32_t instanceIndex,
                        InstanceType const &info,
                        UpdaterFct<InstanceType> update = nullptr);
    bool getInstance(uint32_t instanceIndex, InstanceType &info) const;
    void update(UpdaterFct<InstanceType> update) const;

  private:
    uint32_t instance_index = 1;
    uint32_t _max_instance_nb{};
    uint32_t _current_instance_nb{};
    std::unordered_map<uint32_t, uint32_t> _index_to_buffer_pairing;
    std::vector<InstanceType> _instance_info;
    std::vector<uint32_t> _instance_index;
};

template<typename InstanceType>
void
IndexedBuffer<InstanceType>::setMaxInstanceNb(uint32_t maxInstanceNb)
{
    auto prev_max_model_nb = _max_instance_nb;
    _max_instance_nb = maxInstanceNb;
    _instance_info.reserve(maxInstanceNb);
    _instance_index.reserve(maxInstanceNb);

    if (prev_max_model_nb > _max_instance_nb) {
        for (auto &it : _index_to_buffer_pairing) {
            if (it.second >= maxInstanceNb) {
                _index_to_buffer_pairing.erase(it.first);
            }
        }
        _instance_index.shrink_to_fit();
        _instance_info.shrink_to_fit();
    }
}

template<typename InstanceType>
uint32_t
IndexedBuffer<InstanceType>::getCurrentInstanceNb() const
{
    return (_current_instance_nb);
}

template<typename InstanceType>
uint32_t
IndexedBuffer<InstanceType>::getMaxInstanceNb() const
{
    return (_max_instance_nb);
}

template<typename InstanceType>
uint32_t
IndexedBuffer<InstanceType>::addInstance(InstanceType const &info,
                                         UpdaterFct<InstanceType> update)
{
    if (_current_instance_nb >= _max_instance_nb) {
        return (0);
    }

    _index_to_buffer_pairing.insert({ instance_index, _current_instance_nb });
    _instance_info.emplace_back(info);
    _instance_index.emplace_back(_current_instance_nb);
    if (update) {
        update(_current_instance_nb, info);
    }
    ++_current_instance_nb;
    ++instance_index;
    if (!instance_index) {
        instance_index = 1;
    }
    return (instance_index);
}

template<typename InstanceType>
bool
IndexedBuffer<InstanceType>::removeInstance(uint32_t instanceIndex,
                                            UpdaterFct<InstanceType> update)
{
    if (!_index_to_buffer_pairing.contains(instanceIndex)) {
        return (false);
    }

    auto bufferIndex = _index_to_buffer_pairing[instanceIndex];
    auto infoIt = _instance_info.begin() + bufferIndex;
    auto indexIt = _instance_index.begin() + bufferIndex;
    _index_to_buffer_pairing.erase(instanceIndex);
    _index_to_buffer_pairing.insert_or_assign(*indexIt, bufferIndex);
    *infoIt = _instance_info.back();
    *indexIt = bufferIndex;
    _instance_info.pop_back();
    _instance_index.pop_back();
    --_current_instance_nb;
    if (update) {
        update(bufferIndex, _instance_info[bufferIndex]);
    }
    return (true);
}

template<typename InstanceType>
bool
IndexedBuffer<InstanceType>::updateInstance(uint32_t instanceIndex,
                                            InstanceType const &info,
                                            UpdaterFct<InstanceType> update)
{
    if (!_index_to_buffer_pairing.contains(instanceIndex)) {
        return (false);
    }

    auto bufferIndex = _index_to_buffer_pairing[instanceIndex];
    _instance_info[bufferIndex] = info;
    if (update) {
        update(_index_to_buffer_pairing[bufferIndex], info);
    }
    return (true);
}

template<typename InstanceType>
bool
IndexedBuffer<InstanceType>::getInstance(uint32_t instanceIndex,
                                         InstanceType &info) const
{
    if (!_index_to_buffer_pairing.contains(instanceIndex)) {
        return (false);
    }

    auto bufferIndex = _index_to_buffer_pairing.at(instanceIndex);
    info = _instance_info[bufferIndex];
    return (true);
}

template<typename InstanceType>
void
IndexedBuffer<InstanceType>::update(UpdaterFct<InstanceType> update) const
{
    assert(update);
    for (uint32_t i = 0; i < _current_instance_nb; ++i) {
        update(i, _instance_info[i]);
    }
}

#endif // SCOP_VULKAN_INDEXEDBUFFER_HPP
