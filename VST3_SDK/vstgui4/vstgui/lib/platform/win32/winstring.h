// This file is part of VSTGUI. It is subject to the license terms 
// in the LICENSE file found in the top-level directory of this
// distribution and at http://github.com/steinbergmedia/vstgui/LICENSE

#pragma once

#include "../iplatformstring.h"

#if WINDOWS

#include <windows.h>

namespace VSTGUI {

//-----------------------------------------------------------------------------
class WinString final : public IPlatformString
{
public:
	WinString (UTF8StringPtr utf8String);
	~WinString () noexcept;
	
	void setUTF8String (UTF8StringPtr utf8String) override;

	const WCHAR* getWideString () const { return wideString; }
//-----------------------------------------------------------------------------
protected:
	WCHAR* wideString;
	int wideStringBufferSize;
};

} // VSTGUI

#endif // WINDOWS
