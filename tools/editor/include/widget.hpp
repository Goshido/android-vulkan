#ifndef EDITOR_WIDGET_HPP
#define EDITOR_WIDGET_HPP


namespace editor {

class Widget
{
    public:
        Widget ( Widget const & ) = default;
        Widget &operator = ( Widget const & ) = default;

        Widget ( Widget && ) = default;
        Widget &operator = ( Widget && ) = default;

        virtual void OnButtonDown () noexcept;
        virtual void OnButtonUp () noexcept;
        virtual void OnMouseMove () noexcept;

    protected:
        explicit Widget () = default;
        virtual ~Widget () = default;
};

} // namespace editor


#endif // EDITOR_WIDGET_HPP
