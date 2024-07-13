#!/usr/bin/env dotnet-script
// This is a c# script file

#r "nuget: StateSmith, 0.9.10-alpha" // this line specifies which version of StateSmith to use and download from c# nuget web service.

using StateSmith.Input.Expansions;
using StateSmith.Output.UserConfig;
using StateSmith.Runner;


// Run code generation for TV state machine next
// NOTE!!! Each state machine has its own render config!
SmRunner runner = new(diagramPath: "TvRemote.drawio.svg", new TvRemoteRenderConfig(), transpilerId: TranspilerId.C99);
runner.Settings.stateMachineName = "TvRemoteSm";  // this is needed because the diagram has two state machines in it
runner.Run();


// Run code generation for TV state machine next
// NOTE!!! Each state machine has its own render config!
runner = new(diagramPath: "TvRemote.drawio.svg", new TvRemoteJsRenderConfig(), transpilerId: TranspilerId.JavaScript);
runner.Settings.stateMachineName = "TvRemoteSm";  // this is needed because the diagram has two state machines in it
runner.Run();

// ignore C# guidelines for script stuff below
#pragma warning disable IDE1006, CA1050 

///////////////////////////////////////////////////////////////////////////////////////

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
        #include <stdio.h> // For fprint.
        """;
    
    string IRenderConfigC.CFileExtension => ".c";
    string IRenderConfigC.HFileExtension => ".h";
    string IRenderConfigC.CEnumDeclarer => "typedef enum __attribute__((packed)) {enumName}";

    // Note that StateSmith does not parse the VariableDeclarations string. It outputs whatever is written there. 
    // Use the proper syntax for the output language you are generating for.
    string IRenderConfig.VariableDeclarations => """
        unsigned short volume;     
        unsigned short brightness;   
        unsigned short channel;
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

        string channel_increment() => $"if ({VarsPath}channel > MAX_CHANNEL) {{ {VarsPath}channel = MIN_CHANNEL; }} else {{ {VarsPath}channel++; }}";
        string channel_decrement() => $"if ({VarsPath}channel <= MIN_CHANNEL) {{ {VarsPath}channel = MAX_CHANNEL; }} else {{ {VarsPath}channel--; }}";

        string show(string message) => $"""printf({message} "\n")""";

        string print_volume() => $"""printf("%d\n", {VarsPath}volume)""";
        string print_brightness() => $"""printf("%d\n", {VarsPath}brightness)""";
        string print_channel() => $"""printf("%d\n", {VarsPath}channel)""";
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

        string channel_increment() => $"if ({VarsPath}channel >= MAX_CHANNEL) {{ {VarsPath}channel = MIN_CHANNEL; }} else {{ {VarsPath}channel++; }}";
        string channel_decrement() => $"if ({VarsPath}channel <= MIN_CHANNEL) {{ {VarsPath}channel = MAX_CHANNEL; }} else {{ {VarsPath}channel--; }}";

        string show(string message) => $"console.log({message})";

        string print_volume() => $"console.log({VarsPath}volume)";
        string print_brightness() => $"console.log({VarsPath}brightness)";
        string print_channel() => $"console.log({VarsPath}channel)";
    }
}