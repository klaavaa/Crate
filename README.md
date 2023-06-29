
# Crate (c++20)

Entity Component System

Entities are called ***Crates*** and components ***Attachments***. The entire system is in a header / module  
and everything lives in the crate:: namespace.

Example:
```cpp
#include <iostream>
// You can include crate.hpp traditionally or you can use the module crate.ixx
import crate;

struct PositionAttachment : public crate::IAttachment
{
    PositionAttachment(int x, int y)
        : x(x), y(y)
        {}

    int x;
    int y;
};

void moveObjects(PositionAttachment* posAttachment, int xOffset)
{
    posAttachment->x += xOffset;
}

int main()
{
    uint32_t crateID;
    crate::bind(&crateID); // creates an ID

    // constructs a PositionAttachment bound to crateID
    crate::bindAttachment<PositionAttachment>(&crateID, 20, 15);

    auto attachment = crate::getAttachment<PositionAttachment>(&crateID);
    attachment->x = 35;

    // executes function with every enabled PositionAttachment
    crate::updateAttachments<PositionAttachment>(&moveObjects, 10);

    crate::disableAttachment<PositionAttachment>(&crateID); // you can disable attachments
    crate::enableAttachment<PositionAttachment>(&crateID); // and enable them


    // and you can unbind crates and attachments if you don't have use for them
    crate::unbindAttachment<PositionAttachment>(&crateID);
    crate::unbind(&crateID);
  
}
```
