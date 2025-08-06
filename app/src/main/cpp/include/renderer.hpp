#ifndef ANDROID_VULKAN_RENDERER_HPP
#define ANDROID_VULKAN_RENDERER_HPP


#include <GXCommon/GXMath.hpp>
#include "memory_allocator.hpp"
#include "vulkan_api.hpp"
#include "vulkan_loader.hpp"

GX_DISABLE_COMMON_WARNINGS

#include <map>
#include <set>
#include <span>
#include <vector>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

VK_DEFINE_HANDLE ( WindowHandle )

//----------------------------------------------------------------------------------------------------------------------

class Renderer final
{
    public:
        struct VulkanVersion final
        {
            uint32_t                                        _major = 1U;
            uint32_t                                        _minor = 1U;
            uint32_t                                        _patch = 0U;
        };

    private:
        struct VulkanPhysicalDeviceInfo final
        {
            std::vector<std::string>                        _extensions {};

            VkPhysicalDeviceFeatures                        _features {};
            std::string                                     _deviceName {};
            VkPhysicalDeviceType                            _deviceType = VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM;

            std::vector<std::pair<VkFlags, uint32_t>>       _queueFamilyInfo {};
            VkSurfaceCapabilitiesKHR                        _surfaceCapabilities {};

            explicit VulkanPhysicalDeviceInfo () = default;

            VulkanPhysicalDeviceInfo ( VulkanPhysicalDeviceInfo const & ) = delete;
            VulkanPhysicalDeviceInfo &operator = ( VulkanPhysicalDeviceInfo const & ) = delete;

            VulkanPhysicalDeviceInfo ( VulkanPhysicalDeviceInfo && ) = delete;
            VulkanPhysicalDeviceInfo &operator = ( VulkanPhysicalDeviceInfo && ) = delete;

            ~VulkanPhysicalDeviceInfo () = default;
        };

        using PhysicalDevices = std::map<VkPhysicalDevice, VulkanPhysicalDeviceInfo>;

    private:
        VkFormat                                            _depthImageFormat = VK_FORMAT_UNDEFINED;
        VkFormat                                            _depthStencilImageFormat = VK_FORMAT_UNDEFINED;

        VkDevice                                            _device = VK_NULL_HANDLE;
        std::string_view                                    _deviceName {};
        float                                               _dpi = 96.0F;
        VkInstance                                          _instance = VK_NULL_HANDLE;

        // Note: all reasonable default limits are taken from Vulkan Core spec.
        size_t                                              _descriptorBufferOffsetAlignment = 0U;

        VkExtent3D                                          _maxComputeDispatchSize
        {
            .width = 65535U,
            .height = 65535U,
            .depth = 65535U
        };

        size_t                                              _maxDescriptorSetSampledImages = 96U;
        size_t                                              _maxDescriptorSetStorageBuffers = 24U;
        size_t                                              _maxDescriptorSetStorageImages = 24U;
        float                                               _maxSamplerAnisotropy = 1.0F;
        size_t                                              _maxPerStageResources = 128U;
        size_t                                              _maxUniformBufferRange = 16384U;
        size_t                                              _minStorageBufferOffsetAlignment = 0U;
        size_t                                              _minUniformBufferOffsetAlignment = 0U;
        size_t                                              _nonCoherentAtomSize = 0U;
        VkDeviceSize                                        _samplerDescriptorSize = 0U;
        VkDeviceSize                                        _sampledImageDescriptorSize = 0U;
        VkDeviceSize                                        _storageBufferDescriptorSize = 0U;
        VkDeviceSize                                        _storageImageDescriptorSize = 0U;

        MemoryAllocator                                     _memoryAllocator {};

        VkPhysicalDevice                                    _physicalDevice = VK_NULL_HANDLE;

        VkQueue                                             _queue = VK_NULL_HANDLE;
        uint32_t                                            _queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        VkSurfaceKHR                                        _surface = VK_NULL_HANDLE;
        VkFormat                                            _surfaceFormat = VK_FORMAT_UNDEFINED;

        VkExtent2D                                          _surfaceSize
        {
            .width = 0U,
            .height = 0U
        };

        VkSurfaceTransformFlagBitsKHR                       _surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        VkSwapchainKHR                                      _swapchain = VK_NULL_HANDLE;
        VkSwapchainKHR                                      _oldSwapchain = VK_NULL_HANDLE;

#ifdef AV_ENABLE_VVL

        PFN_vkCreateDebugUtilsMessengerEXT                  vkCreateDebugUtilsMessengerEXT = nullptr;
        PFN_vkDestroyDebugUtilsMessengerEXT                 vkDestroyDebugUtilsMessengerEXT = nullptr;

        VkDebugUtilsMessengerEXT                            _debugUtilsMessenger = VK_NULL_HANDLE;

#endif // AV_ENABLE_VVL

        VkExtent2D                                          _viewportResolution
        {
            .width = 0U,
            .height = 0U
        };

        std::vector<VkPhysicalDeviceGroupProperties>        _physicalDeviceGroups {};
        PhysicalDevices                                     _physicalDeviceInfo {};
        VkPhysicalDeviceMemoryProperties                    _physicalDeviceMemoryProperties {};

        std::vector<VkSurfaceFormatKHR>                     _surfaceFormats {};

        std::vector<VkImage>                                _swapchainImages {};
        std::vector<VkImageView>                            _swapchainImageViews {};

        GXMat4                                              _presentationEngineTransform {};
        bool                                                _vSync = true;
        VulkanLoader                                        _vulkanLoader {};

    public:
        explicit Renderer () = default;

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
        [[nodiscard]] size_t GetDescriptorBufferOffsetAlignment () const noexcept;
        [[nodiscard]] VkDevice GetDevice () const noexcept;
        [[nodiscard]] std::string_view GetDeviceName () const noexcept;
        [[nodiscard]] float GetDPI () const noexcept;
        [[nodiscard]] VkExtent3D const &GetMaxComputeDispatchSize () const noexcept;
        [[nodiscard]] size_t GetMaxDescriptorSetSampledImages () const noexcept;
        [[nodiscard]] size_t GetMaxDescriptorSetStorageBuffers () const noexcept;
        [[nodiscard]] size_t GetMaxDescriptorSetStorageImages () const noexcept;
        [[nodiscard]] float GetMaxSamplerAnisotropy () const noexcept;
        [[nodiscard]] size_t GetMaxPerStageResources () const noexcept;
        [[nodiscard]] size_t GetMaxUniformBufferRange () const noexcept;
        [[nodiscard]] size_t GetMinStorageBufferOffsetAlignment () const noexcept;
        [[nodiscard]] size_t GetMinUniformBufferOffsetAlignment () const noexcept;
        [[nodiscard]] size_t GetNonCoherentAtomSize () const noexcept;

        [[nodiscard]] size_t GetPresentImageCount () const noexcept;
        [[nodiscard]] VkImageView const &GetPresentImageView ( size_t imageIndex ) const noexcept;

        // Note this transform MUST be applied after projection transform to compensate screen orientation on the
        // mobile device. For more information please reference by links:
        // See docs/preprocessor-macros.md#macro-av-native-mode
        [[nodiscard]] GXMat4 const &GetPresentationEngineTransform () const noexcept;

        [[nodiscard]] VkQueue GetQueue () const noexcept;
        [[nodiscard]] uint32_t GetQueueFamilyIndex () const noexcept;
        [[nodiscard]] VkDeviceSize GetSamplerDescriptorSize () const noexcept;
        [[nodiscard]] VkDeviceSize GetSampledImageDescriptorSize () const noexcept;
        [[nodiscard]] VkDeviceSize GetStorageBufferDescriptorSize () const noexcept;
        [[nodiscard]] VkDeviceSize GetStorageImageDescriptorSize () const noexcept;
        [[nodiscard]] VkFormat GetSurfaceFormat () const noexcept;
        [[nodiscard]] VkExtent2D const &GetSurfaceSize () const noexcept;
        [[nodiscard]] VkSwapchainKHR &GetSwapchain () noexcept;

        // This resolution must be used by projection matrices. Resolution takes into consideration
        // current device orientation. The actual presentation image resolution can be acquired
        // by Renderer::GetSurfaceSize API.
        [[nodiscard]] VkExtent2D const &GetViewportResolution () const noexcept;

        [[nodiscard]] bool GetVSync () const noexcept;

        [[nodiscard]] bool OnCreateSwapchain ( WindowHandle nativeWindow, bool vSync ) noexcept;
        void OnDestroySwapchain ( bool preserveSurface ) noexcept;

        [[nodiscard]] bool OnCreateDevice ( std::string_view const &userGPU ) noexcept;
        void OnDestroyDevice () noexcept;

        void OnSetDPI ( float dpi ) noexcept;

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

        [[nodiscard]] static bool CheckExtensionCommon ( std::set<std::string> const &allExtensions,
            char const* extension
        ) noexcept;

        // Method returns true if "result" equals VK_SUCCESS. Otherwise method returns false.
        [[nodiscard]] static bool CheckVkResult ( VkResult result, char const* from, char const* message ) noexcept;

        [[nodiscard]] static VkImageAspectFlags ResolveImageViewAspect ( VkFormat format ) noexcept;
        [[nodiscard]] static char const* ResolveVkFormat ( VkFormat format ) noexcept;

#ifdef AV_ENABLE_VVL

        [[nodiscard]] static VkBool32 VKAPI_PTR OnVulkanDebugUtils (
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageTypes,
            VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
            void* pUserData
        );

#endif // AV_ENABLE_VVL

    private:
        [[nodiscard]] bool CheckRequiredFeatures ( std::vector<std::string> const &deviceExtensions ) noexcept;
        [[nodiscard]] bool CheckRequiredFormats () noexcept;

#ifdef AV_ENABLE_VVL

        [[nodiscard]] bool DeployDebugFeatures () noexcept;
        void DestroyDebugFeatures () noexcept;

#endif // AV_ENABLE_VVL

        [[nodiscard]] bool DeployDevice ( std::string_view const &userGPU ) noexcept;
        void DestroyDevice () noexcept;

        [[nodiscard]] bool DeployInstance () noexcept;
        void DestroyInstance () noexcept;

        [[nodiscard]] bool DeploySurface ( WindowHandle nativeWindow ) noexcept;
        [[nodiscard]] bool DeployNativeSurface ( WindowHandle nativeWindow ) noexcept;
        void DestroySurface () noexcept;

        [[nodiscard]] bool DeploySwapchain ( bool vSync ) noexcept;
        void DestroySwapchain ( bool preserveSurface ) noexcept;

        void GetPlatformFeatureProperties () noexcept;

        [[nodiscard]] bool PrintPhysicalDeviceExtensionInfo ( VkPhysicalDevice physicalDevice ) noexcept;
        void PrintPhysicalDeviceFeatureInfo ( VkPhysicalDevice physicalDevice ) noexcept;

        void PrintPhysicalDeviceLimits ( VkPhysicalDeviceLimits const &limits ) noexcept;
        void PrintPhysicalDeviceMemoryProperties ( VkPhysicalDevice physicalDevice ) noexcept;
        [[nodiscard]] bool PrintPhysicalDeviceInfo ( uint32_t deviceIndex, VkPhysicalDevice physicalDevice ) noexcept;

        [[nodiscard]] bool SelectTargetCompositeAlpha (
            VkCompositeAlphaFlagBitsKHR &targetCompositeAlpha
        ) const noexcept;

        [[nodiscard]] bool SelectTargetHardware ( std::string_view const &userGPU ) noexcept;
        [[nodiscard]] bool SelectTargetPresentMode ( VkPresentModeKHR &targetPresentMode, bool vSync ) const noexcept;
        [[nodiscard]] bool SelectTargetSurfaceFormat ( VkColorSpaceKHR &targetColorSpace ) noexcept;

        [[nodiscard]] static bool CheckFeature ( VkBool32 feature, char const* name ) noexcept;
        [[nodiscard]] static std::span<char const* const> GetDeviceExtensions () noexcept;
        [[nodiscard]] static std::span<char const* const> GetInstanceExtensions () noexcept;
        [[nodiscard]] static std::span<std::pair<VkFormat, char const* const> const> GetRequiredFormats () noexcept;
        [[nodiscard]] static VkPhysicalDeviceFeatures2 GetRequiredPhysicalDeviceFeatures () noexcept;
        [[nodiscard]] static VulkanVersion GetRequiredVulkanVersion () noexcept;

        [[nodiscard]] static bool PrintCoreExtensions () noexcept;
        static void PrintFloatProp ( char const* indent, char const* name, float value ) noexcept;
        static void PrintFloatVec2Prop ( char const* indent, char const* name, float const value[] ) noexcept;
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
            std::span<std::pair<uint32_t, char const*> const> flagSet
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
            std::span<std::pair<uint32_t, char const*> const> flagSet
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_RENDERER_HPP
