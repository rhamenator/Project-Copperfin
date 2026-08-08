# Third-Party Notices

Project Copperfin includes the following separately licensed material. These
notices do not change the GPL-3.0-only-with-exception terms for first-party
Project Copperfin code.

## Vendored Ed25519 Verification Code

Location: `src/licensing/third_party/ed25519_ref/`

The following notice is reproduced from that directory's `license.txt`:

> Copyright (c) 2015 Orson Peters <orsonpeters@gmail.com>
>
> This software is provided 'as-is', without any express or implied warranty.
> In no event will the authors be held liable for any damages arising from the
> use of this software.
>
> Permission is granted to anyone to use this software for any purpose,
> including commercial applications, and to alter it and redistribute it
> freely, subject to the following restrictions:
>
> 1. The origin of this software must not be misrepresented; you must not claim
>    that you wrote the original software. If you use this software in a
>    product, an acknowledgment in the product documentation would be
>    appreciated but is not required.
> 2. Altered source versions must be plainly marked as such, and must not be
>    misrepresented as being the original software.
> 3. This notice may not be removed or altered from any source distribution.

Project Copperfin uses only the verification path. The locally trimmed public
header is plainly marked as modified; the remaining vendored implementation
files are maintained for direct comparison with their source distribution.
