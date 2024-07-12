#!/usr/bin/env dotnet-script
// This is a c# script file

#r "nuget: StateSmith, 0.9.10-alpha" // this line specifies which version of StateSmith to use and download from c# nuget web service.

using StateSmith.Input.Expansions;
using StateSmith.Output.UserConfig;
using StateSmith.Runner;

// Run code generation for button first
// NOTE!!! Each state machine has its own render config!
SmRunner runner = new(diagramPath: "TvRemote.drawio.svg", new ButtonRenderConfig(), transpilerId: TranspilerId.C99);
runner.Settings.stateMachineName = "ButtonSm";  // this is needed because the diagram has two state machines in it
runner.Run();

// Run code generation for TV state machine next
// NOTE!!! Each state machine has its own render config!
runner = new(diagramPath: "TvRemote.drawio.svg", new TvRemoteRenderConfig(), transpilerId: TranspilerId.C99);
runner.Settings.stateMachineName = "TvRemoteSm";  // this is needed because the diagram has two state machines in it
runner.Run();

// Run code generation for JS button
// NOTE!!! Each state machine has its own render config!
runner = new(diagramPath: "TvRemote.drawio.svg", new ButtonJsRenderConfig(), transpilerId: TranspilerId.JavaScript);
runner.Settings.stateMachineName = "ButtonSm";  // this is needed because the diagram has two state machines in it
runner.Run();

// Run code generation for TV state machine next
// NOTE!!! Each state machine has its own render config!
runner = new(diagramPath: "TvRemote.drawio.svg", new TvRemoteJsRenderConfig(), transpilerId: TranspilerId.JavaScript);
runner.Settings.stateMachineName = "TvRemoteSm";  // this is needed because the diagram has two state machines in it
runner.Run();

// ignore C# guidelines for script stuff below
#pragma warning disable IDE1006, CA1050 

///////////////////////////////////////////////////////////////////////////////////////

// See https://github.com/StateSmith/tutorial-2/tree/main/lesson-2
public class ButtonRenderConfig : IRenderConfigC
{
    string IRenderConfigC.CFileTop => """
        // Constants used by the code.
        const unsigned int LONG_PRESS_TIMEOUT = 800;
        const unsigned int DEBOUNCE_TIMEOUT = 100;


    """;
    string IRenderConfigC.CFileIncludes => """
        #include "Arduino.h"
        """;
    
    string IRenderConfigC.CFileExtension => ".c";
    string IRenderConfigC.HFileExtension => ".h";
    string IRenderConfigC.CEnumDeclarer => "typedef enum __attribute__((packed)) {enumName}";

    string IRenderConfig.VariableDeclarations => """
        // Note! This example below uses bit fields just to show that you can. They aren't required.

        // This can be made to be 11 bits if RAM is at a premium. See laser tag menu example.
        uint32_t timer_started_at_ms;

        unsigned short input_is_pressed : 1;     // input
        unsigned short output_event_press : 1;   // output
        unsigned short output_event_release : 1; // output
        unsigned short output_event_long : 1;    // output
        """;

    // See https://github.com/StateSmith/tutorial-2/tree/main/lesson-3
    public class Expansions : UserExpansionScriptBase
    {
        // inputs
        public string is_pressed => VarsPath + "input_is_pressed";
        public string is_released => $"(!{is_pressed})";

        // outputs
        public string output_event(string eventName) => $"{VarsPath}output_event_{eventName.ToLower()} = true";

        // time stuff
        public string now_ms => $"millis()";   // directly calls Arduino C++ code
        public string timer_started_at_ms => AutoVarName();
        public string timer_ms => $"({now_ms} - {timer_started_at_ms})";   // unsigned math works even with ms roll over
        public string reset_timer() => $"{timer_started_at_ms} = {now_ms}";

        public string is_debounced => $"({timer_ms} >= DEBOUNCE_TIMEOUT)";
    }
}

// This class gives StateSmith the info it needs to generate working code. This class can have any name.
public class TvRemoteRenderConfig : IRenderConfigC
{
    // `CFileTop` text will appear at the top of the generated source file. Use for comments, copyright notices, code...
    string IRenderConfigC.CFileTop => """
        // TV Remote project for F&P Healthcare.

        // Constants used by the code.
        const unsigned short MAX_BRIGHTNESS = 100;
        const unsigned short MIN_BRIGHTNESS = 0;
        const unsigned short MAX_VOLUME = 100;
        const unsigned short MIN_VOLUME = 0;
        const unsigned short MAX_CHANNEL = 256;
        const unsigned short MIN_CHANNEL = 1;


        """;

    string IRenderConfigC.CFileIncludes => """
        #include "Arduino.h"
        """;
    
    string IRenderConfigC.CFileExtension => ".c";
    string IRenderConfigC.HFileExtension => ".h";
    string IRenderConfigC.CEnumDeclarer => "typedef enum __attribute__((packed)) {enumName}";

    // Note that StateSmith does not parse the VariableDeclarations string. It outputs whatever is written there. 
    // Use the proper syntax for the output language you are generating for.
    string IRenderConfig.VariableDeclarations => """
        unsigned short volume = 50;     
        unsigned short brightness = 50;   
        unsigned short channel = 1;
        """;

    public class TvRemoteExpansions : UserExpansionScriptBase
    {
        string volume() => AutoVarName();
        string brightness() => AutoVarName();
        string channel() => AutoVarName();


        string volume_increment() => $"if ({VarsPath}volume < MAX_VOLUME) {{ {VarsPath}volume++; }}";
        string volume_decrement() => $"if ({VarsPath}volume > MIN_VOLUME) {{ {VarsPath}volume--; }}";

        
        string brightness_increment() => $"if ({VarsPath}brightness < MAX_BRIGHTNESS) {{ {VarsPath}brightness++; }}";
        string brightness_decrement() => $"if ({VarsPath}brightness > MIN_BRIGHTNESS) {{ {VarsPath}brightness--; }}";

        string channel_increment() => $"{VarsPath}channel++; if ({VarsPath}channel > MAX_CHANNEL) {{ {VarsPath}channel = MIN_CHANNEL; }}";
        string channel_decrement() => $"{VarsPath}channel--; if ({VarsPath}channel < MIN_CHANNEL) {{ {VarsPath}channel = MAX_CHANNEL; }}";

        string show(string message) => $"""printf({message})""";

        string print_volume() => $"printf({VarsPath}volume)";
        string print_brightness() => $"printf({VarsPath}brightness)";
        string print_channel() => $"printf({VarsPath}channel)";
    }
}


public class ButtonJsRenderConfig : IRenderConfig
{

    string IRenderConfig.FileTop => """
    
        // Constants used by the code.
        const LONG_PRESS_TIMEOUT = 800;
        const DEBOUNCE_TIMEOUT = 100;

    """;

    string IRenderConfig.VariableDeclarations => """
        timer_started_at_ms: 0,

        input_is_pressed: false,     // input
        output_event_press: false,   // output
        output_event_release: false, // output
        output_event_long: false,    // output
        """;

    // See https://github.com/StateSmith/tutorial-2/tree/main/lesson-3
    public class MyExpansions : UserExpansionScriptBase
    {
        // inputs
        public string is_pressed => VarsPath + "input_is_pressed";
        public string is_released => $"(!!({is_pressed}))";

        // outputs
        // public string output_event(string eventName) => $"{VarsPath}output_event_{eventName.ToLower()} = true";
        public string output_event(string eventName) => $"{VarsPath}output_event_{eventName.ToLower()} = true; console.log('output_event_{eventName.ToLower()}')";

        // time stuff
        public string now_ms => $"Date.now().valueOf()";
        public string timer_started_at_ms => AutoVarName();
        public string timer_ms => $"({now_ms} - {timer_started_at_ms})";   // unsigned math works even with ms roll over
        public string reset_timer() => $"{timer_started_at_ms} = {now_ms}";

        public string is_debounced => $"(!!({timer_ms} >= DEBOUNCE_TIMEOUT))";
    }
}

public class TvRemoteJsRenderConfig : IRenderConfig
{
    // `FileTop` text will appear at the top of the generated source file. Use for comments, copyright notices, code...
    string IRenderConfig.FileTop => """
        // Constants used by the code.
        const MAX_BRIGHTNESS = 100;
        const MIN_BRIGHTNESS = 0;
        const MAX_VOLUME = 100;
        const MIN_VOLUME = 0;
        const MAX_CHANNEL = 256;
        const MIN_CHANNEL = 1;


        """;

    // Note that StateSmith does not parse the VariableDeclarations string. It outputs whatever is written there. 
    // Use the proper syntax for the output language you are generating for.
    string IRenderConfig.VariableDeclarations => """
        volume: 50,     
        brightness: 50,   
        channel: 1,
        """;

    public class TvRemoteExpansions : UserExpansionScriptBase
    {
        string volume() => AutoVarName();
        string brightness() => AutoVarName();
        string channel() => AutoVarName();


        string volume_increment() => $"if ({VarsPath}volume < MAX_VOLUME) {{ {VarsPath}volume++; }}";
        string volume_decrement() => $"if ({VarsPath}volume > MIN_VOLUME) {{ {VarsPath}volume--; }}";

        
        string brightness_increment() => $"if ({VarsPath}brightness < MAX_BRIGHTNESS) {{ {VarsPath}brightness++; }}";
        string brightness_decrement() => $"if ({VarsPath}brightness > MIN_BRIGHTNESS) {{ {VarsPath}brightness--; }}";

        string channel_increment() => $"{VarsPath}channel++; if ({VarsPath}channel > MAX_CHANNEL) {{ {VarsPath}channel = MIN_CHANNEL; }}";
        string channel_decrement() => $"{VarsPath}channel--; if ({VarsPath}channel < MIN_CHANNEL) {{ {VarsPath}channel = MAX_CHANNEL; }}";

        string show(string message) => $"console.log({message})";

        string print_volume() => $"console.log({VarsPath}volume)";
        string print_brightness() => $"console.log({VarsPath}brightness)";
        string print_channel() => $"console.log({VarsPath}channel)";
    }
}