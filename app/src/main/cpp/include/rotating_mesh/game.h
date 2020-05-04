#ifndef ROTATING_MESH_GAME_H
#define ROTATING_MESH_GAME_H


#include <game.h>
#include "texture2D.h"


namespace rotating_mesh {

class Game final : public android_vulkan::Game
{
    private:
        VkCommandPool                   _commandPool;

        Texture2D                       _material1Diffuse;
        Texture2D                       _material2Diffuse;
        Texture2D                       _material2Normal;
        Texture2D                       _material3Diffuse;
        Texture2D                       _material3Normal;

        //VkBuffer                        _constantBuffer;
        //VkDeviceMemory                  _constantBufferDeviceMemory;

        //VkDescriptorPool                _descriptorPool;
        VkDescriptorSetLayout           _descriptorSetLayout;

        //VkBuffer                        _mesh;
        //VkDeviceMemory                  _meshDeviceMemory;

        VkPipeline                      _pipeline;
        VkPipelineLayout                _pipelineLayout;

        VkFence                         _presentationImageFence;
        VkRenderPass                    _renderPass;
        VkSemaphore                     _renderPassEndSemaphore;

        VkSampler                       _sampler09Mips;
        VkSampler                       _sampler10Mips;
        VkSampler                       _sampler11Mips;

        VkShaderModule                  _vertexShaderModule;
        VkShaderModule                  _fragmentShaderModule;

        //std::vector<VkCommandBuffer>    _commandBuffers;

    public:
        Game ();
        ~Game () override = default;

        Game ( const Game &other ) = delete;
        Game& operator = ( const Game &other ) = delete;

    private:
        bool IsReady () override;

        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

        bool BeginFrame ( uint32_t &presentationImageIndex, android_vulkan::Renderer &renderer );
        bool EndFrame ( uint32_t presentationImageIndex, android_vulkan::Renderer &renderer );

        bool CreateCommandPool ( android_vulkan::Renderer &renderer );
        void DestroyCommandPool ( android_vulkan::Renderer &renderer );

        bool CreateConstantBuffer ( android_vulkan::Renderer &renderer );
        void DestroyConstantBuffer ( android_vulkan::Renderer &renderer );

        bool CreateMesh ( android_vulkan::Renderer &renderer );
        void DestroyMesh ( android_vulkan::Renderer &renderer );

        bool CreatePipeline ( android_vulkan::Renderer &renderer );
        void DestroyPipeline ( android_vulkan::Renderer &renderer );

        bool CreatePipelineLayout ( android_vulkan::Renderer &renderer );
        void DestroyPipelineLayout ( android_vulkan::Renderer &renderer );

        bool CreateRenderPass ( android_vulkan::Renderer &renderer );
        void DestroyRenderPass ( android_vulkan::Renderer &renderer );

        bool CreateSamplers ( android_vulkan::Renderer &renderer );
        void DestroySamplers ( android_vulkan::Renderer &renderer );

        bool CreateShaderModules ( android_vulkan::Renderer &renderer );
        void DestroyShaderModules ( android_vulkan::Renderer &renderer );

        bool CreateSyncPrimitives ( android_vulkan::Renderer &renderer );
        void DestroySyncPrimitives ( android_vulkan::Renderer &renderer );

        bool CreateTextures ( android_vulkan::Renderer &renderer );
        void DestroyTextures ( android_vulkan::Renderer &renderer );

        bool InitCommandBuffers ( android_vulkan::Renderer &renderer );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_H
