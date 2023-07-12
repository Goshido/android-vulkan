#ifndef ANDROID_VULKAN_RENDERER_HPP
#define ANDROID_VULKAN_RENDERER_HPP


#include <GXCommon/GXMath.hpp>
#include "logger.hpp"
#include "memory_allocator.hpp"
#include "vulkan_api.hpp"
#include "vulkan_loader.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <map>
#include <set>
#include <vector>
#include <android/native_window.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

struct VulkanPhysicalDeviceInfo final
{
    std::vector<char>                               _extensionStorage;

    std::vector<char const*>                        _extensions;
    VkPhysicalDeviceFeatures                        _features;

    std::vector<std::pair<VkFlags, uint32_t>>       _queueFamilyInfo;
    VkSurfaceCapabilitiesKHR                        _surfaceCapabilities;

    VulkanPhysicalDeviceInfo () noexcept;

    VulkanPhysicalDeviceInfo ( VulkanPhysicalDeviceInfo const & ) = delete;
    VulkanPhysicalDeviceInfo &operator = ( VulkanPhysicalDeviceInfo const & ) = delete;

    ~VulkanPhysicalDeviceInfo () = default;
};

//----------------------------------------------------------------------------------------------------------------------

class Renderer final
{
    private:
        using LogType = void ( * ) ( char const* format, ... );

    private:
        VkFormat                                                            _depthImageFormat;
        VkFormat                                                            _depthStencilImageFormat;
        VkDevice                                                            _device;
        float                                                               _dpi;
        VkInstance                                                          _instance;

        bool                                                                _isDeviceExtensionChecked;
        bool                                                                _isDeviceExtensionSupported;

        size_t                                                              _maxUniformBufferRange;
        MemoryAllocator                                                     _memoryAllocator;

        VkPhysicalDevice                                                    _physicalDevice;

        VkQueue                                                             _queue;
        uint32_t                                                            _queueFamilyIndex;

        VkSurfaceKHR                                                        _surface;
        VkFormat                                                            _surfaceFormat;
        VkExtent2D                                                          _surfaceSize;
        VkSurfaceTransformFlagBitsKHR                                       _surfaceTransform;
        VkSwapchainKHR                                                      _swapchain;

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        PFN_vkCreateDebugUtilsMessengerEXT                                  vkCreateDebugUtilsMessengerEXT;
        PFN_vkDestroyDebugUtilsMessengerEXT                                 vkDestroyDebugUtilsMessengerEXT;

        VkDebugUtilsMessengerEXT                                            _debugUtilsMessenger;
        VkDebugUtilsMessengerCreateInfoEXT                                  _debugUtilsMessengerCreateInfo;

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        VkExtent2D                                                          _viewportResolution;

        std::vector<VkPhysicalDeviceGroupProperties>                        _physicalDeviceGroups;
        std::map<VkPhysicalDevice, VulkanPhysicalDeviceInfo>                _physicalDeviceInfo;
        VkPhysicalDeviceMemoryProperties                                    _physicalDeviceMemoryProperties;

        std::vector<VkSurfaceFormatKHR>                                     _surfaceFormats;

        std::vector<VkImage>                                                _swapchainImages;
        std::vector<VkImageView>                                            _swapchainImageViews;

        GXMat4                                                              _presentationEngineTransform;
        VulkanLoader                                                        _vulkanLoader;

        static std::map<VkColorSpaceKHR, char const*> const                 _vulkanColorSpaceMap;
        static std::map<VkCompositeAlphaFlagBitsKHR, char const*> const     _vulkanCompositeAlphaMap;
        static std::map<VkFormat, char const*> const                        _vulkanFormatMap;
        static std::map<VkPhysicalDeviceType, char const*> const            _vulkanPhysicalDeviceTypeMap;
        static std::map<VkPresentModeKHR, char const*> const                _vulkanPresentModeMap;
        static std::map<VkResult, char const*> const                        _vulkanResultMap;
        static std::map<VkSurfaceTransformFlagsKHR, char const*> const      _vulkanSurfaceTransformMap;

    public:
        Renderer () noexcept;

        Renderer ( Renderer const & ) = delete;
        Renderer &operator = ( Renderer const & ) = delete;

        Renderer ( Renderer && ) = delete;
        Renderer &operator = ( Renderer && ) = delete;

        ~Renderer () = default;

        // Method returns true if swapchain does not change. User code can safely render frames.
        // Otherwise method returns false.
        [[nodiscard]] bool CheckSwapchainStatus () noexcept;

        [[nodiscard]] bool CreateShader ( VkShaderModule &shader,
            std::string &&shaderFile,
            char const* errorMessage
        ) const noexcept;

        [[nodiscard]] bool FinishAllJobs () noexcept;

        [[nodiscard]] VkFormat GetDefaultDepthFormat () const noexcept;
        [[nodiscard]] VkFormat GetDefaultDepthStencilFormat () const noexcept;
        [[nodiscard]] VkDevice GetDevice () const noexcept;
        [[nodiscard]] float GetDPI () const noexcept;
        [[nodiscard]] size_t GetMaxUniformBufferRange () const noexcept;

        [[nodiscard]] size_t GetPresentImageCount () const noexcept;
        [[nodiscard]] VkImageView const &GetPresentImageView ( size_t imageIndex ) const noexcept;

        // Note this transform MUST be applied after projection transform to compensate screen orientation on the
        // mobile device. For more information please reference by links:
        // https://community.arm.com/developer/tools-software/graphics/b/blog/posts/appropriate-use-of-surface-rotation
        // https://github.com/KhronosGroup/Vulkan-Samples/blob/master/samples/performance/surface_rotation/surface_rotation_tutorial.md
        [[nodiscard]] GXMat4 const &GetPresentationEngineTransform () const noexcept;

        [[nodiscard]] VkQueue GetQueue () const noexcept;
        [[nodiscard]] uint32_t GetQueueFamilyIndex () const noexcept;
        [[nodiscard]] VkFormat GetSurfaceFormat () const noexcept;
        [[nodiscard]] VkExtent2D const &GetSurfaceSize () const noexcept;
        [[nodiscard]] VkSwapchainKHR &GetSwapchain () noexcept;

        // This resolution must be used by projection matrices. Resolution takes into consideration
        // current device orientation. The actual presentation image resolution can be acquired
        // by Renderer::GetSurfaceSize API.
        [[nodiscard]] VkExtent2D const &GetViewportResolution () const noexcept;

        [[nodiscard]] bool OnCreateSwapchain ( ANativeWindow &nativeWindow, bool vSync ) noexcept;
        void OnDestroySwapchain () noexcept;

        [[nodiscard]] bool OnCreateDevice ( float dpi ) noexcept;
        void OnDestroyDevice () noexcept;

        [[nodiscard]] bool TryAllocateMemory ( VkDeviceMemory &memory,
            VkDeviceSize &offset,
            VkMemoryRequirements const &requirements,
            VkMemoryPropertyFlags memoryProperties,
            char const* errorMessage
        ) noexcept;

        void FreeMemory ( VkDeviceMemory memory, VkDeviceSize offset ) noexcept;

        [[nodiscard]] bool MapMemory ( void* &ptr,
            VkDeviceMemory memory,
            VkDeviceSize offset,
            char const* from,
            char const* message
        ) noexcept;

        void UnmapMemory ( VkDeviceMemory memory ) noexcept;

        // See docs/vulkan-memory-view.md
        [[maybe_unused]] void MakeVulkanMemorySnapshot () noexcept;

        // Method returns true if "result" equals VK_SUCCESS. Otherwise method returns false.
        [[nodiscard]] static bool CheckVkResult ( VkResult result, char const* from, char const* message ) noexcept;

        [[nodiscard]] static VkImageAspectFlags ResolveImageViewAspect ( VkFormat format ) noexcept;
        [[nodiscard]] static char const* ResolveVkFormat ( VkFormat format ) noexcept;

    private:
        [[nodiscard]] bool CheckExtensionShaderFloat16Int8 ( std::set<std::string> const &allExtensions ) noexcept;
        [[nodiscard]] bool CheckRequiredDeviceExtensions ( std::vector<char const*> const &deviceExtensions ) noexcept;

        // "features" is an array of offsets inside VkPhysicalDeviceFeatures structure.
        [[nodiscard]] bool CheckRequiredFeatures ( VkPhysicalDevice physicalDevice,
            size_t const* features,
            size_t count
        ) noexcept;

        [[nodiscard]] bool CheckRequiredFormats () noexcept;

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] bool DeployDebugFeatures () noexcept;
        void DestroyDebugFeatures () noexcept;

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] bool DeployDevice () noexcept;
        void DestroyDevice () noexcept;

        [[nodiscard]] bool DeployInstance () noexcept;
        void DestroyInstance () noexcept;

        [[nodiscard]] bool DeploySurface ( ANativeWindow &nativeWindow ) noexcept;
        void DestroySurface () noexcept;

        [[nodiscard]] bool DeploySwapchain ( bool vSync ) noexcept;
        void DestroySwapchain () noexcept;

        [[nodiscard]] bool PrintPhysicalDeviceExtensionInfo ( VkPhysicalDevice physicalDevice ) noexcept;
        void PrintPhysicalDeviceFeatureInfo ( VkPhysicalDevice physicalDevice ) noexcept;

        void PrintPhysicalDeviceLimits ( VkPhysicalDeviceLimits const &limits ) noexcept;
        void PrintPhysicalDeviceMemoryProperties ( VkPhysicalDevice physicalDevice ) noexcept;
        [[nodiscard]] bool PrintPhysicalDeviceInfo ( uint32_t deviceIndex, VkPhysicalDevice physicalDevice ) noexcept;

        [[nodiscard]] bool SelectTargetCompositeAlpha (
            VkCompositeAlphaFlagBitsKHR &targetCompositeAlpha
        ) const noexcept;

        [[nodiscard]] bool SelectTargetHardware ( VkPhysicalDevice &targetPhysicalDevice,
            uint32_t &targetQueueFamilyIndex
        ) const noexcept;

        [[nodiscard]] bool SelectTargetPresentMode ( VkPresentModeKHR &targetPresentMode, bool vSync ) const noexcept;

        [[nodiscard]] bool SelectTargetSurfaceFormat ( VkFormat &targetColorFormat,
            VkColorSpaceKHR &targetColorSpace,
            VkFormat &targetDepthFormat,
            VkFormat &targetDepthStencilFormat
        ) const noexcept;

        [[nodiscard]] static bool CheckExtensionCommon ( std::set<std::string> const &allExtensions,
            char const* extension
        ) noexcept;

#ifdef ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] static VkBool32 VKAPI_PTR OnVulkanDebugUtils (
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
            void* pUserData
        );

#endif // ANDROID_VULKAN_ENABLE_VULKAN_VALIDATION_LAYERS

        [[nodiscard]] static bool PrintCoreExtensions () noexcept;
        static void PrintFloatProp ( char const* indent, char const* name, float value ) noexcept;
        static void PrintFloatVec2Prop ( char const* indent, char const* name, const float value[] ) noexcept;
        static void PrintINT32Prop ( char const* indent, char const* name, int32_t value ) noexcept;
        [[nodiscard]] static bool PrintInstanceLayerInfo () noexcept;
        static void PrintPhysicalDeviceCommonProps ( VkPhysicalDeviceProperties const &props ) noexcept;

        static void PrintPhysicalDeviceGroupInfo ( uint32_t groupIndex,
                VkPhysicalDeviceGroupProperties const &props
        ) noexcept;

        [[nodiscard]] static bool PrintPhysicalDeviceLayerInfo ( VkPhysicalDevice physicalDevice ) noexcept;

        static void PrintPhysicalDeviceQueueFamilyInfo ( uint32_t queueFamilyIndex,
            VkQueueFamilyProperties const &props
        ) noexcept;

        static void PrintPhysicalDeviceSparse ( VkPhysicalDeviceSparseProperties const &sparse ) noexcept;
        static void PrintSizeProp ( char const* indent, char const* name, size_t value ) noexcept;
        static void PrintUINT32Prop ( char const* indent, char const* name, uint32_t value ) noexcept;
        static void PrintUINT32Vec2Prop ( char const* indent, char const* name, uint32_t const value[] ) noexcept;
        static void PrintUINT32Vec3Prop ( char const* indent, char const* name, uint32_t const value[] ) noexcept;
        static void PrintUTF8Prop ( char const* indent, char const* name, char const* value ) noexcept;
        static void PrintVkBool32Prop ( char const* indent, char const* name, VkBool32 value ) noexcept;
        static void PrintVkExtent2DProp ( char const* indent, char const* name, VkExtent2D const &value ) noexcept;
        static void PrintVkExtent3DProp ( char const* indent, char const* name, VkExtent3D const &value ) noexcept;

        static void PrintVkExtensionProp ( uint32_t extensionIndex,
            char const* category,
            VkExtensionProperties const &extension
        ) noexcept;

        static void PrintVkFlagsProp ( char const* indent,
            char const* name,
            VkFlags flags,
            size_t flagSetCount,
            std::pair<uint32_t, char const*> const flagSet[]
        ) noexcept;

        static void PrintVkHandler ( char const* indent, char const* name, void* handler ) noexcept;
        static void PrintVkLayerProperties ( uint32_t layerIndex, VkLayerProperties const &layer ) noexcept;
        static void PrintVkPresentModeProp ( uint32_t modeIndex, VkPresentModeKHR mode ) noexcept;
        static void PrintVkSurfaceCapabilities ( VkSurfaceCapabilitiesKHR const &caps ) noexcept;
        static void PrintVkSurfaceFormatKHRProp ( uint32_t formatIndex, VkSurfaceFormatKHR const &format ) noexcept;
        static void PrintVkVersion ( char const* indent, char const* name, uint32_t version ) noexcept;

        [[nodiscard]] static char const* ResolvePhysicalDeviceType ( VkPhysicalDeviceType type ) noexcept;
        [[nodiscard]] static char const* ResolveVkColorSpaceKHR ( VkColorSpaceKHR colorSpace ) noexcept;

        [[nodiscard]] static char const* ResolveVkCompositeAlpha (
            VkCompositeAlphaFlagBitsKHR compositeAlpha
        ) noexcept;

        [[nodiscard]] static char const* ResolveVkPresentModeKHR ( VkPresentModeKHR mode ) noexcept;
        [[nodiscard]] static char const* ResolveVkResult ( VkResult result ) noexcept;
        [[nodiscard]] static char const* ResolveVkSurfaceTransform ( VkSurfaceTransformFlagsKHR transform ) noexcept;

        [[nodiscard]] static std::string StringifyVkFlags ( VkFlags flags,
            size_t flagSetCount,
            std::pair<uint32_t, char const*> const flagSet[]
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_RENDERER_HPP
