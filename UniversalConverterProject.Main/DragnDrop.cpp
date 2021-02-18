#include "DragnDrop.h"

void PatchDragnDrop() {
    patch::RedirectCall(0x45D327, EntityScreenConstructor<FmEntityType::Player, 0x5CA7C0>);
    patch::RedirectCall(0x45D217, EntityScreenConstructor<FmEntityType::YouthPlayer, 0x5E70C0>);
    patch::RedirectCall(0xD30047, EntityScreenConstructor<FmEntityType::Manager, 0x5C26F0>);
    patch::RedirectCall(0x69AFE7, EntityScreenConstructor<FmEntityType::Staff, 0x69AE60>);
    patch::RedirectCall(0x56F5C7, EntityScreenConstructor<FmEntityType::Club, 0x65B080>);
    patch::RedirectCall(0x673F77, EntityScreenConstructor<FmEntityType::League, 0x5EF330>);
    patch::RedirectCall(0xD30BE7, EntityScreenConstructor<FmEntityType::Country, 0x5F4370>);

    patch::RedirectCall(0x5CBF23, EntityScreenDestructor<FmEntityType::Player, 0x5CA8E0>);
    patch::RedirectCall(0x5E7183, EntityScreenDestructor<FmEntityType::YouthPlayer, 0x5E6300>);
    patch::RedirectCall(0x5C2543, EntityScreenDestructor<FmEntityType::Manager, 0x5C13A0>);
    patch::RedirectCall(0x69ADE3, EntityScreenDestructor<FmEntityType::Staff, 0x69A8D0>);
    patch::RedirectCall(0x65AFC3, EntityScreenDestructor<FmEntityType::Club, 0x65AEB0>);
    patch::RedirectCall(0x5EF223, EntityScreenDestructor<FmEntityType::League, 0x5EF180>);
    patch::RedirectCall(0x5F48C9, EntityScreenDestructor<FmEntityType::Country, 0xD54220>);
    patch::RedirectJump(0x5F43B6, EntityScreenDestructor<FmEntityType::Country, 0xD54220>);
}
