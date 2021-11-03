#ifndef _H3_INTERNAL_H_
#define _H3_INTERNAL_H_

#include <h3/common.h>
#include <string>

void               H3Internal_SetAssertMessage(const std::string& msg);
const std::string& H3Internal_GetAssertMessage();

void               H3Internal_HandleAssertPopup();

#endif  /* _H3_INTERNAL_H_ */
