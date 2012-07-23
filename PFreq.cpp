//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("PFreq.res");
USEFORM("MainScreen.cpp", MainForm);
USEUNIT("Utils.cpp");
USEUNIT("GaussSmth.cpp");
USEUNIT("Savgol.cpp");
USEUNIT("convlv.cpp");
USEUNIT("GraphImage.cpp");
USEFORM("Background.cpp", BackgroundForm);
USEFORM("SelOutFile.cpp", OutputFile);
USEFORM("SelColumn.cpp", SelectColumn);
USEFORM("XRange.cpp", XRangeForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->CreateForm(__classid(TMainForm), &MainForm);
         Application->CreateForm(__classid(TBackgroundForm), &BackgroundForm);
         Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------
