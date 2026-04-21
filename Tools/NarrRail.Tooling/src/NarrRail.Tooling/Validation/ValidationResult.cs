namespace NarrRail.Tooling.Validation;

/// <summary>
/// Validation error severity
/// </summary>
public enum ValidationSeverity
{
    Error,   // Must fix
    Warning  // Should fix
}

/// <summary>
/// Validation result
/// </summary>
public class ValidationResult
{
    public bool IsValid => Errors.Count == 0;
    public List<ValidationError> Errors { get; set; } = new();
    public List<ValidationError> Warnings { get; set; } = new();

    public void AddError(string message, string? location = null, int? line = null, int? column = null)
    {
        Errors.Add(new ValidationError
        {
            Severity = ValidationSeverity.Error,
            Message = message,
            Location = location,
            Line = line,
            Column = column
        });
    }

    public void AddWarning(string message, string? location = null, int? line = null, int? column = null)
    {
        Warnings.Add(new ValidationError
        {
            Severity = ValidationSeverity.Warning,
            Message = message,
            Location = location,
            Line = line,
            Column = column
        });
    }

    public void PrintToConsole()
    {
        if (IsValid && Warnings.Count == 0)
        {
            Console.ForegroundColor = ConsoleColor.Green;
            Console.WriteLine("✓ Validation passed");
            Console.ResetColor();
            return;
        }

        foreach (var error in Errors)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Write("ERROR: ");
            Console.ResetColor();
            Console.WriteLine(error.ToString());
        }

        foreach (var warning in Warnings)
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.Write("WARNING: ");
            Console.ResetColor();
            Console.WriteLine(warning.ToString());
        }

        Console.WriteLine();
        if (Errors.Count > 0)
        {
            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine($"✗ Validation failed with {Errors.Count} error(s) and {Warnings.Count} warning(s)");
            Console.ResetColor();
        }
        else
        {
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"⚠ Validation passed with {Warnings.Count} warning(s)");
            Console.ResetColor();
        }
    }
}

/// <summary>
/// Validation error
/// </summary>
public class ValidationError
{
    public ValidationSeverity Severity { get; set; }
    public string Message { get; set; } = string.Empty;
    public string? Location { get; set; }
    public int? Line { get; set; }
    public int? Column { get; set; }

    public override string ToString()
    {
        var parts = new List<string>();

        if (!string.IsNullOrEmpty(Location))
            parts.Add($"[{Location}]");

        if (Line.HasValue)
            parts.Add($"Line {Line}");

        if (Column.HasValue)
            parts.Add($"Col {Column}");

        var locationStr = parts.Count > 0 ? string.Join(" ", parts) + ": " : "";
        return $"{locationStr}{Message}";
    }
}
