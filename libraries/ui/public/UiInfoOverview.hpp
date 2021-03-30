#ifndef SCOP_VULKAN_INFO_OVERVIEW_HPP
#define SCOP_VULKAN_INFO_OVERVIEW_HPP

#include <string>

class UiInfoOverview final
{
  public:
    UiInfoOverview() = default;
    ~UiInfoOverview() = default;
    UiInfoOverview(UiInfoOverview const &src) = delete;
    UiInfoOverview &operator=(UiInfoOverview const &rhs) = delete;
    UiInfoOverview(UiInfoOverview &&src) = delete;
    UiInfoOverview &operator=(UiInfoOverview &&rhs) = delete;

    void draw(bool &fps, bool &model_info);
    void setAvgFps(float avgFps);
    void setCurrentFps(float currentFps);
    void setModelInfo(uint32_t nbVertices,
                      uint32_t nbIndices,
                      uint32_t nbFaces);

  private:
    float _avg_fps{};
    float _current_fps{};
    uint32_t _nb_vertices{};
    uint32_t _nb_indices{};
    uint32_t _nb_faces{};
};

#endif // SCOP_VULKAN_INFO_OVERVIEW_HPP
