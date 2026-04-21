using System.CommandLine;
using NarrRail.Tooling.Parsing;
using NarrRail.Tooling.Validation;

namespace NarrRail.Tooling;

class Program
{
    static async Task<int> Main(string[] args)
    {
        var rootCommand = new RootCommand("NarrRail script tooling CLI");

        // Validate command
        var validateCommand = new Command("validate", "Validate NarrRail script files");
        var pathArgument = new Argument<string>("path", "Path to script file or directory");
        var recursiveOption = new Option<bool>(new[] { "--recursive", "-r" }, "Recursively validate all scripts in directory");

        validateCommand.AddArgument(pathArgument);
        validateCommand.AddOption(recursiveOption);
        validateCommand.SetHandler((path, recursive) =>
        {
            return Task.FromResult(ValidateHandler(path, recursive));
        }, pathArgument, recursiveOption);

        rootCommand.AddCommand(validateCommand);

        return await rootCommand.InvokeAsync(args);
    }

    static int ValidateHandler(string path, bool recursive)
    {
        try
        {
            var parser = new ScriptParser();
            var validator = new ScriptValidator();

            if (File.Exists(path))
            {
                // Single file validation
                return ValidateSingleFile(path, parser, validator);
            }
            else if (Directory.Exists(path))
            {
                // Directory validation
                return ValidateDirectory(path, recursive, parser, validator);
            }
            else
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"Error: Path not found: {path}");
                Console.ResetColor();
                return 1;
            }
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"Fatal error: {ex.Message}");
            Console.ResetColor();
            return 1;
        }
    }

    static int ValidateSingleFile(string filePath, ScriptParser parser, ScriptValidator validator)
    {
        Console.WriteLine($"Validating: {filePath}");
        Console.WriteLine();

        try
        {
            var script = parser.ParseFile(filePath);
            var result = validator.Validate(script);

            result.PrintToConsole();

            return result.IsValid ? 0 : 1;
        }
        catch (Exception ex)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"ERROR: Failed to parse file: {ex.Message}");
            Console.ResetColor();
            return 1;
        }
    }

    static int ValidateDirectory(string dirPath, bool recursive, ScriptParser parser, ScriptValidator validator)
    {
        var searchOption = recursive ? SearchOption.AllDirectories : SearchOption.TopDirectoryOnly;
        var files = Directory.GetFiles(dirPath, "*.narrrail.yaml", searchOption)
            .Concat(Directory.GetFiles(dirPath, "*.narrrail.yml", searchOption))
            .ToList();

        if (files.Count == 0)
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"No .narrrail.yaml or .narrrail.yml files found in: {dirPath}");
            Console.ResetColor();
            return 0;
        }

        Console.WriteLine($"Found {files.Count} script file(s) in: {dirPath}");
        Console.WriteLine();

        int totalFiles = 0;
        int validFiles = 0;
        int invalidFiles = 0;
        int totalErrors = 0;
        int totalWarnings = 0;

        foreach (var file in files)
        {
            totalFiles++;
            Console.WriteLine($"[{totalFiles}/{files.Count}] {Path.GetFileName(file)}");

            try
            {
                var script = parser.ParseFile(file);
                var result = validator.Validate(script);

                if (result.IsValid)
                {
                    validFiles++;
                    Console.ForegroundColor = ConsoleColor.Green;
                    Console.WriteLine("  ✓ Valid");
                    Console.ResetColor();
                }
                else
                {
                    invalidFiles++;
                    Console.ForegroundColor = ConsoleColor.Red;
                    Console.WriteLine($"  ✗ Invalid ({result.Errors.Count} error(s))");
                    Console.ResetColor();

                    foreach (var error in result.Errors.Take(3))
                    {
                        Console.WriteLine($"    - {error.Message}");
                    }

                    if (result.Errors.Count > 3)
                    {
                        Console.WriteLine($"    ... and {result.Errors.Count - 3} more error(s)");
                    }
                }

                totalErrors += result.Errors.Count;
                totalWarnings += result.Warnings.Count;
            }
            catch (Exception ex)
            {
                invalidFiles++;
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine($"  ✗ Parse error: {ex.Message}");
                Console.ResetColor();
            }

            Console.WriteLine();
        }

        // Summary
        Console.WriteLine("=".PadRight(50, '='));
        Console.WriteLine($"Total files: {totalFiles}");
        Console.ForegroundColor = ConsoleColor.Green;
        Console.WriteLine($"Valid: {validFiles}");
        Console.ResetColor();

        if (invalidFiles > 0)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"Invalid: {invalidFiles}");
            Console.ResetColor();
        }

        Console.WriteLine($"Total errors: {totalErrors}");
        Console.WriteLine($"Total warnings: {totalWarnings}");

        return invalidFiles > 0 ? 1 : 0;
    }
}
