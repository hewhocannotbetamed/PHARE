
#ifndef PHARE_MHD_MESSENGER_INFO_H
#define PHARE_MHD_MESSENGER_INFO_H

#include "messenger_info.h"



namespace PHARE
{
namespace amr
{
    class MHDMessengerInfo : public IMessengerInfo
    {
    public:
        virtual ~MHDMessengerInfo() = default;
    };

} // namespace amr


} // namespace PHARE
#endif
