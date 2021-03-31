#ifndef SCOP_VULKAN_UIOPENMODEL_HPP
#define SCOP_VULKAN_UIOPENMODEL_HPP

#include <string>

class UiOpenModel final
{
  public:
    UiOpenModel() = default;
    ~UiOpenModel() = default;
    UiOpenModel(UiOpenModel const &src) = delete;
    UiOpenModel &operator=(UiOpenModel const &rhs) = delete;
    UiOpenModel(UiOpenModel &&src) = delete;
    UiOpenModel &operator=(UiOpenModel &&rhs) = delete;

    bool drawFilepathWindow(bool &open);
    void drawErrorWindow(bool &open);
    [[nodiscard]] std::string getModelFilepath() const;

  private:
    char _filepath[4096] = { 0 };
    bool _force_focus = true;
};

#endif // SCOP_VULKAN_UIOPENMODEL_HPP
