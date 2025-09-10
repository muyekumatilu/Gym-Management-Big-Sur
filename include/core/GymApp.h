#ifndef GYM_APP_H
#define GYM_APP_H

#include <wx/wx.h>

class GymApp : public wxApp {
public:
    virtual bool OnInit();
    virtual int OnExit();
};

#endif // GYM_APP_H