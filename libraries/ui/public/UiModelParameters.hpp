#ifndef SCOP_VULKAN_UIMODELPARAMETERS_HPP
#define SCOP_VULKAN_UIMODELPARAMETERS_HPP

class UiModelParameters final
{
  public:
    UiModelParameters() = default;
    ~UiModelParameters() = default;
    UiModelParameters(UiModelParameters const &src) = delete;
    UiModelParameters &operator=(UiModelParameters const &rhs) = delete;
    UiModelParameters(UiModelParameters &&src) = delete;
    UiModelParameters &operator=(UiModelParameters &&rhs) = delete;

    bool draw(bool &open);
    void resetAllParams();

    [[nodiscard]] float getYaw() const;
    [[nodiscard]] float getPitch() const;
    [[nodiscard]] float getRoll() const;
    [[nodiscard]] float getScale() const;

  private:
    float _yaw{};
    float _pitch{};
    float _roll{};
    float _scale = 1.0f;
};

#endif // SCOP_VULKAN_UIMODELPARAMETERS_HPP
