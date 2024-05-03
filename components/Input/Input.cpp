#include "Input.h"

namespace GameUtils
{
    void Input::SetPressed(bool isPressed)
    {
        m_isPressed = isPressed;
    }

    bool Input::GetPressed()
    {
        return m_isPressed;
    }
}