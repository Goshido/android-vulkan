#ifndef PBR_COMPONENT_H
#define PBR_COMPONENT_H


#include "render_session.h"


namespace pbr {

class Component
{
    public:
        Component ( Component const &other ) = delete;
        Component& operator = ( Component const &other ) = delete;

        Component ( Component &&other ) = delete;
        Component& operator = ( Component &&other ) = delete;

        virtual ~Component () = default;

        virtual void Submit ( RenderSession &renderSession ) = 0;

    protected:
        Component () = default;
};

} // namespace pbr


#endif // PBR_COMPONENT_H
