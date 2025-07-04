#ifndef EDITOR_ACTION_HPP
#define EDITOR_ACTION_HPP


namespace editor {

class Action
{
    public:
        explicit Action () = default;

        Action ( Action const & ) = delete;
        Action &operator = ( Action const & ) = delete;

        Action ( Action && ) = default;
        Action &operator = ( Action && ) = default;

        virtual ~Action () = default;

        virtual void Redo () noexcept;
        virtual void Undo () noexcept;
};

} // namespace editor


#endif // EDITOR_ACTION_HPP
