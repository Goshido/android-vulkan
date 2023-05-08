#ifndef PBR_UI_ELEMENT_H
#define PBR_UI_ELEMENT_H





namespace pbr {

class UIElement
{
    private:


    public:
        UIElement ( UIElement const & ) = delete;
        UIElement& operator = ( UIElement const & ) = delete;
    
        UIElement ( UIElement && ) = delete;
        UIElement& operator = ( UIElement && ) = delete;
    
        virtual ~UIElement () = default;
    
    protected:
        UIElement () = default;
};

} // namespace pbr


#endif // PBR_UI_ELEMENT_H
