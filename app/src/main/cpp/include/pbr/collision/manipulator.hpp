#ifndef PBR_COLLISION_MANIPULATOR_HPP
#define PBR_COLLISION_MANIPULATOR_HPP


#include <rigid_body.hpp>


namespace pbr::collision {

class Manipulator final
{
    private:
        android_vulkan::RigidBodyRef    _body {};
        int8_t                          _height = 0;
        int8_t                          _pitch = 0;
        int8_t                          _roll = 0;

    public:
        Manipulator () = default;

        Manipulator ( Manipulator const & ) = delete;
        Manipulator &operator = ( Manipulator const & ) = delete;

        Manipulator ( Manipulator && ) = delete;
        Manipulator &operator = ( Manipulator && ) = delete;

        ~Manipulator () = default;

        void Capture ( android_vulkan::RigidBodyRef &body ) noexcept;
        void Free () noexcept;
        void Update ( GXMat4 const &cameraLocal, float deltaTime ) noexcept;

    private:
        static void OnBDown ( void* context ) noexcept;
        static void OnBUp ( void* context ) noexcept;
        static void OnDownDown ( void* context ) noexcept;
        static void OnDownUp ( void* context ) noexcept;
        static void OnLeftDown ( void* context ) noexcept;
        static void OnLeftUp ( void* context ) noexcept;
        static void OnRightDown ( void* context ) noexcept;
        static void OnRightUp ( void* context ) noexcept;
        static void OnUpDown ( void* context ) noexcept;
        static void OnUpUp ( void* context ) noexcept;
        static void OnYDown ( void* context ) noexcept;
        static void OnYUp ( void* context ) noexcept;
};

} // namespace pbr::collision


#endif // PBR_COLLISION_MANIPULATOR_HPP
