#include "MobilePhone.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"

using namespace plugin;

const UInt WidgetHandyStructSize = 0x55C;

struct PhoneExtension {
	void *ImgSmartphone;
	void *BtTheme;
};

void ApplyMobilePhoneTheme(void *screen, PhoneExtension *ext) {
	if (Settings::GetInstance().PhoneTheme < 1 || Settings::GetInstance().PhoneTheme > Settings::GetInstance().PhoneNumThemes)
		Settings::GetInstance().PhoneTheme = 1;
	SetImageFilename(ext->ImgSmartphone, Format(L"art_fm\\screens\\MobilePhone\\Phone%u.tga", Settings::GetInstance().PhoneTheme));
}

void METHOD WidgetHandy_Setup(void *screen) {
	PhoneExtension *ext = raw_ptr<PhoneExtension>(screen, WidgetHandyStructSize);
	ext->ImgSmartphone = CallMethodAndReturn<void *, 0xD44380>(screen, "Trfm1|_ImgSmartphone");
	ext->BtTheme = CallMethodAndReturn<void *, 0xD44360>(screen, "Trfm1|BtTheme");
	ApplyMobilePhoneTheme(screen, ext);
	CallMethod<0x9E9CB0>(screen);
}

void *METHOD WidgetHandy_ButtonReleased(void *screen, DUMMY_ARG, UInt *pId, UInt a2) {
	PhoneExtension *ext = raw_ptr<PhoneExtension>(screen, WidgetHandyStructSize);
	if (*pId == CallVirtualMethodAndReturn<UInt, 23>(ext->BtTheme)) {
		Settings::GetInstance().PhoneTheme += 1;
		ApplyMobilePhoneTheme(screen, ext);
	}
	return CallMethodAndReturn<void *, 0x9E9780>(screen, pId, a2);
}

UInt GetPhoneNewsDateColor(UInt) {
	return 0xFFffa69d;
}

void PatchMobilePhone(FM::Version v) {
	if (v.id() == ID_FM_13_1030_RLD) {
		patch::SetUInt(0x9E9834 + 1, WidgetHandyStructSize + sizeof(PhoneExtension));
		patch::SetUInt(0x9E983B + 1, WidgetHandyStructSize + sizeof(PhoneExtension));
		patch::RedirectCall(0x9E9E99, WidgetHandy_Setup);
		patch::SetPointer(0x2440514, WidgetHandy_ButtonReleased);
		//patch::SetUChar(0xA1A87C + 1, 14); // gui color (red) for news date
		patch::RedirectCall(0xA1A889, GetPhoneNewsDateColor);
	}
}
