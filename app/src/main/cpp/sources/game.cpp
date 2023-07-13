#include <game.hpp>


namespace android_vulkan {

bool Game::OnInitSoundSystem () noexcept
{
    return true;
}

void Game::OnDestroySoundSystem () noexcept
{
    // NOTHING
}

} // namespace android_vulkan
