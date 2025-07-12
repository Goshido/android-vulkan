#ifndef EDITOR_SOUND_EMITTER_COMPONENT_HPP
#define EDITOR_SOUND_EMITTER_COMPONENT_HPP


#include "component.hpp"


namespace editor {

class SoundEmitterComponent final : public Component
{
    public:
        constexpr static std::string_view       TYPE = "SoundEmitter";

    public:
        explicit SoundEmitterComponent () noexcept;

        SoundEmitterComponent ( SoundEmitterComponent const & ) = delete;
        SoundEmitterComponent &operator = ( SoundEmitterComponent const & ) = delete;

        SoundEmitterComponent ( SoundEmitterComponent && ) = delete;
        SoundEmitterComponent &operator = ( SoundEmitterComponent && ) = delete;

        explicit SoundEmitterComponent ( SaveState::Container const &info ) noexcept;

        ~SoundEmitterComponent () = default;

    private:
        void Save ( SaveState::Container &root ) const noexcept override;
};

} // namespace editor


#endif // EDITOR_SOUND_EMITTER_COMPONENT_HPP
