
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef FWD_DIALOG_HPP_INCLUDED
#define FWD_DIALOG_HPP_INCLUDED

class Dialog;

enum DialogEntry
{
	DIALOG_TEXT = 1,
	DIALOG_LINK = 2
};

enum DialogReply
{
	DIALOG_REPLY_OK = 1,
	DIALOG_REPLY_LINK = 2,
};

#endif // FWD_DIALOG_HPP_INCLUDED
