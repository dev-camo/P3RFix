#Requires -Version 7.2
param (
    [ValidatePattern('^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$')]
    [string]$Version
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Keep this manifest explicit: adding or removing a source notice requires review.
# Loader-related sections are retained in both packages; only standalone bundles
# the loader binary itself.
$LicenseSections = @(
    @{ Title = 'P3RFix license'; Path = 'LICENSE.md' }
    @{ Title = 'Third-party attribution and provenance'; Path = 'THIRD_PARTY_NOTICES.md' }
    @{ Title = 'Compiled dependency: inipp'; Path = 'licenses/inipp/LICENSE.txt' }
    @{ Title = 'Compiled dependency: spdlog'; Path = 'licenses/spdlog/LICENSE' }
    @{ Title = 'Compiled dependency: fmt'; Path = 'licenses/fmt/LICENSE' }
    @{ Title = 'Compiled dependency: SafetyHook'; Path = 'licenses/SafetyHook/LICENSE' }
    @{ Title = 'Compiled dependency: Zydis'; Path = 'licenses/Zydis/LICENSE' }
    @{ Title = 'Compiled dependency: Zycore'; Path = 'licenses/Zycore/LICENSE' }
    @{ Title = 'Retained Unreal support: UnrealContainers'; Path = 'licenses/UnrealContainers/LICENSE' }
    @{ Title = 'Standalone loader: Ultimate ASI Loader'; Path = 'licenses/Ultimate-ASI-Loader/LICENSE' }
    @{ Title = 'Standalone loader dependency: miniz'; Path = 'licenses/Ultimate-ASI-Loader/miniz-LICENSE' }
    @{ Title = 'Standalone loader dependency: MinHook and Hacker Disassembler Engine'; Path = 'licenses/Ultimate-ASI-Loader/MinHook-LICENSE.txt' }
    @{ Title = 'Standalone loader dependency: injector utility'; Path = 'licenses/Ultimate-ASI-Loader/injector-utility-LICENSE.txt' }
)

function Read-LicenseText {
    param (
        [string]$RepositoryRoot,
        [string]$RelativePath
    )

    $InputPath = Join-Path $RepositoryRoot $RelativePath
    if (-not (Test-Path -LiteralPath $InputPath -PathType Leaf)) {
        throw "Missing license input: $RelativePath"
    }

    try {
        # Strict decoding reports damaged input rather than replacing author names.
        $Utf8 = [System.Text.UTF8Encoding]::new($false, $true)
        $Text = $Utf8.GetString([System.IO.File]::ReadAllBytes($InputPath))
    } catch {
        throw "Could not read UTF-8 license input '$RelativePath': $($_.Exception.Message)"
    }
    if ($Text.StartsWith([string][char]0xFEFF, [System.StringComparison]::Ordinal)) {
        $Text = $Text.Substring(1)
    }
    if ([string]::IsNullOrWhiteSpace($Text)) {
        throw "Empty license input: $RelativePath"
    }

    # Normalize only line endings. Preserve wording, indentation, and whitespace.
    return $Text.Replace("`r`n", "`n").Replace("`r", "`n").Replace("`n", "`r`n")
}

function Write-CombinedLicenseFile {
    param (
        [string]$RepositoryRoot,
        [object[]]$Sections,
        [string]$DestinationPath
    )

    $InputPaths = [System.Collections.Generic.HashSet[string]]::new(
        [System.StringComparer]::OrdinalIgnoreCase)
    $Titles = [System.Collections.Generic.HashSet[string]]::new(
        [System.StringComparer]::Ordinal)
    $Inputs = @()
    foreach ($Section in $Sections) {
        if (-not $InputPaths.Add($Section.Path)) {
            throw "Duplicate license input in manifest: $($Section.Path)"
        }
        if (-not $Titles.Add($Section.Title)) {
            throw "Duplicate license section title: $($Section.Title)"
        }
        $Inputs += @{
            Title = $Section.Title
            Path = $Section.Path
            Text = Read-LicenseText -RepositoryRoot $RepositoryRoot -RelativePath $Section.Path
        }
    }

    # Enumeration is only for completeness checking; it never determines order.
    $LicenseDirectory = Join-Path $RepositoryRoot 'licenses'
    if (-not (Test-Path -LiteralPath $LicenseDirectory -PathType Container)) {
        throw 'Missing license input directory: licenses'
    }
    $ActualPaths = [System.Collections.Generic.HashSet[string]]::new(
        [System.StringComparer]::OrdinalIgnoreCase)
    foreach ($File in Get-ChildItem -LiteralPath $LicenseDirectory -File -Recurse -Force) {
        $RelativePath = [System.IO.Path]::GetRelativePath($RepositoryRoot, $File.FullName).
            Replace('\', '/')
        [void]$ActualPaths.Add($RelativePath)
        if (-not $InputPaths.Contains($RelativePath)) {
            throw "License input is not listed in the manifest: $RelativePath"
        }
    }
    foreach ($InputPath in $InputPaths) {
        if ($InputPath.StartsWith('licenses/', [System.StringComparison]::OrdinalIgnoreCase) -and
            -not $ActualPaths.Contains($InputPath)) {
            throw "Manifest license input is absent from licenses inventory: $InputPath"
        }
    }

    $Newline = "`r`n"
    $Document = [System.Text.StringBuilder]::new()
    [void]$Document.Append("P3RFix release licenses and notices$Newline$Newline")
    [void]$Document.Append("This document collects P3RFix's license and the full notices for components$Newline")
    [void]$Document.Append("used or credited by this release. Each component retains its own terms.$Newline")
    [void]$Document.Append("Ultimate ASI Loader and its dependencies describe the loader bundled only$Newline")
    [void]$Document.Append("with the standalone ZIP. The Reloaded-II ZIP does not include or require$Newline")
    [void]$Document.Append("the standalone loader binary, dsound.dll.$Newline$Newline")

    $Payloads = @()
    foreach ($SectionInput in $Inputs) {
        $Heading = "======================================================================$Newline" +
            "$($SectionInput.Title)$Newline" +
            "Source repository path: $($SectionInput.Path)$Newline$Newline"
        [void]$Document.Append($Heading)
        if ($SectionInput.Path -eq 'THIRD_PARTY_NOTICES.md') {
            [void]$Document.Append("Repository-relative links in the notice below refer to the source checkout.$Newline")
            [void]$Document.Append("The full referenced license texts follow in this same document.$Newline$Newline")
        } elseif ($SectionInput.Path.StartsWith('licenses/Ultimate-ASI-Loader/',
            [System.StringComparison]::OrdinalIgnoreCase)) {
            [void]$Document.Append("This loader component is bundled only with the standalone ZIP.$Newline")
            [void]$Document.Append("The Reloaded-II ZIP does not include the standalone loader binary.$Newline$Newline")
        }

        $Payloads += @{
            Heading = $Heading
            Path = $SectionInput.Path
            Offset = $Document.Length
            Length = $SectionInput.Text.Length
        }
        [void]$Document.Append($SectionInput.Text)
        if (-not $SectionInput.Text.EndsWith($Newline, [System.StringComparison]::Ordinal)) {
            [void]$Document.Append($Newline)
        }
        [void]$Document.Append($Newline)
    }

    $Utf8 = [System.Text.UTF8Encoding]::new($false, $true)
    [System.IO.File]::WriteAllText($DestinationPath, $Document.ToString(), $Utf8)

    # Verify the written document, including every complete payload in order.
    $WrittenBytes = [System.IO.File]::ReadAllBytes($DestinationPath)
    $HasEncodingMarker = $WrittenBytes.Length -ge 3 -and
        $WrittenBytes[0] -eq 0xEF -and $WrittenBytes[1] -eq 0xBB -and $WrittenBytes[2] -eq 0xBF
    if ($WrittenBytes.Length -eq 0 -or $HasEncodingMarker) {
        throw "Combined licensing document is empty or has an encoding marker: $DestinationPath"
    }
    $WrittenText = $Utf8.GetString($WrittenBytes)
    $PreviousHeading = -1
    foreach ($Payload in $Payloads) {
        $HeadingOffset = $WrittenText.IndexOf($Payload.Heading, [System.StringComparison]::Ordinal)
        if ($HeadingOffset -le $PreviousHeading -or
            $WrittenText.IndexOf($Payload.Heading, $HeadingOffset + 1,
                [System.StringComparison]::Ordinal) -ne -1) {
            throw "Combined licensing section is missing, duplicated, or out of order: $($Payload.Path)"
        }
        $ExpectedText = Read-LicenseText -RepositoryRoot $RepositoryRoot -RelativePath $Payload.Path
        if ($WrittenText.Substring($Payload.Offset, $Payload.Length) -cne $ExpectedText) {
            throw "Combined licensing section does not preserve its input: $($Payload.Path)"
        }
        $PreviousHeading = $HeadingOffset
    }
    if (-not $WrittenText.EndsWith($Newline, [System.StringComparison]::Ordinal)) {
        throw "Combined licensing document has no final newline: $DestinationPath"
    }
    Write-Information "Combined licensing document: all $($Inputs.Count) source documents included." -InformationAction Continue
}

function Test-ByteArraysEqual {
    param (
        [byte[]]$First,
        [byte[]]$Second
    )

    if ($First.Length -ne $Second.Length) {
        return $false
    }
    for ($Index = 0; $Index -lt $First.Length; $Index++) {
        if ($First[$Index] -ne $Second[$Index]) {
            return $false
        }
    }
    return $true
}

function Assert-PackageContent {
    param (
        [string]$ArchivePath,
        [byte[]]$ReferenceLicenses,
        [bool]$Standalone
    )

    $Archive = [System.IO.Compression.ZipFile]::OpenRead($ArchivePath)
    try {
        $Entries = @{}
        $LicenseEntries = @()
        $LoaderEntries = @()
        foreach ($Entry in $Archive.Entries) {
            $Name = $Entry.FullName.Replace('\', '/')
            if ($Name -match '(^|/)licenses/' -or
                $Name -ieq 'LICENSE.md' -or $Name -ieq 'THIRD_PARTY_NOTICES.md') {
                throw "Archive '$ArchivePath' contains a legacy licensing entry: $Name"
            }
            if ($Name.EndsWith('/')) {
                continue
            }
            if ($Entries.ContainsKey($Name)) {
                throw "Archive '$ArchivePath' contains a duplicate entry: $Name"
            }
            $Entries[$Name] = $Entry
            $FileName = ($Name -split '/')[-1]
            if ($FileName -ieq 'LICENSES') {
                $LicenseEntries += $Name
            }
            if ($FileName -ieq 'dsound.dll' -or $FileName -ieq 'dinput8.dll') {
                $LoaderEntries += $Name
            }
        }

        if ($LicenseEntries.Count -ne 1 -or $LicenseEntries[0] -ine 'LICENSES') {
            throw "Archive '$ArchivePath' must contain exactly one LICENSES file at its existing root."
        }
        $RequiredEntries = @('P3RFix.asi', 'P3RFix.ini', 'LICENSES')
        if ($Standalone) {
            $RequiredEntries += 'dsound.dll'
            if ($LoaderEntries.Count -ne 1 -or $LoaderEntries[0] -ine 'dsound.dll') {
                throw "Archive '$ArchivePath' must contain its standalone loader only at dsound.dll."
            }
        } else {
            $RequiredEntries += 'ModConfig.json'
            if ($LoaderEntries.Count -ne 0) {
                throw "Archive '$ArchivePath' must not contain the standalone loader."
            }
        }
        foreach ($Name in $RequiredEntries) {
            if (-not $Entries.ContainsKey($Name) -or $Entries[$Name].Length -eq 0) {
                throw "Archive '$ArchivePath' is missing a nonempty required root entry: $Name"
            }
        }

        $EntryStream = $Entries['LICENSES'].Open()
        $Memory = [System.IO.MemoryStream]::new()
        try {
            $EntryStream.CopyTo($Memory)
            $ArchivedLicenses = $Memory.ToArray()
        } finally {
            $Memory.Dispose()
            $EntryStream.Dispose()
        }
        if (-not (Test-ByteArraysEqual -First $ArchivedLicenses -Second $ReferenceLicenses)) {
            throw "Archive '$ArchivePath' has LICENSES bytes that differ from the generated reference."
        }
        Write-Information "$(Split-Path $ArchivePath -Leaf): one complete LICENSES; package destinations verified." -InformationAction Continue
        return ,$ArchivedLicenses
    } finally {
        $Archive.Dispose()
    }
}

if (-not $Version) {
    throw 'Pass a release version, for example: ./create_release.ps1 -Version 1.4.0'
}
if (-not $IsWindows) {
    throw 'Release builds require Windows with Visual Studio 2022, CMake, and xmake.'
}

# Pin the loader binary and verify it before including it in a release.
# https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/tag/v9.7.4
$LoaderVersion = 'v9.7.4'
$LoaderArchive = 'Ultimate-ASI-Loader-NoPDB_x64.zip'
$LoaderSha256 = 'e5860e7d9a1805267535b65749575b5e406cc6ea3325c7392189c578815045d1'
$ZipName = "P3RFix_${Version}.zip"

Push-Location $PSScriptRoot
try {
    $BuildDirectory = Join-Path $PSScriptRoot 'build'
    $StagingDirectory = Join-Path $BuildDirectory 'package-staging'
    $DownloadDirectory = Join-Path $BuildDirectory 'release-inputs'
    $StandaloneDirectory = Join-Path $StagingDirectory 'standalone'
    $ReloadedDirectory = Join-Path $StagingDirectory 'reloaded'
    $LoaderDirectory = Join-Path $StagingDirectory 'loader'
    $CombinedLicensePath = Join-Path $StagingDirectory 'LICENSES'
    $ZipPath = Join-Path $BuildDirectory $ZipName
    $ReloadedZipPath = Join-Path $BuildDirectory 'P3RFix_Reloaded-II.zip'
    $ReleaseBodyPath = Join-Path $BuildDirectory 'release_body.md'

    foreach ($RequiredPath in @('P3RFix.ini', 'assets/r2-package/ModConfig.json',
        'LICENSE.md', 'THIRD_PARTY_NOTICES.md', 'licenses',
        'licenses/Ultimate-ASI-Loader/LICENSE', 'release_body.md')) {
        if (-not (Test-Path -LiteralPath $RequiredPath)) {
            throw "Missing release input: $RequiredPath"
        }
    }

    if (Test-Path -LiteralPath $StagingDirectory) {
        Remove-Item -LiteralPath $StagingDirectory -Recurse -Force
    }
    New-Item -ItemType Directory -Path $StandaloneDirectory, $ReloadedDirectory,
        $LoaderDirectory, $DownloadDirectory -Force | Out-Null
    Write-CombinedLicenseFile -RepositoryRoot $PSScriptRoot -Sections $LicenseSections `
        -DestinationPath $CombinedLicensePath

    foreach ($OutputPath in @($ZipPath, $ReloadedZipPath, $ReleaseBodyPath)) {
        if (Test-Path -LiteralPath $OutputPath) {
            Remove-Item -LiteralPath $OutputPath -Force
        }
    }

    Write-Information "Building P3RFix $Version for Windows x64..." -InformationAction Continue
    & xmake f -y -p windows -a x64 -m release "--fix_version=$Version"
    if ($LASTEXITCODE -ne 0) { throw "xmake configuration failed ($LASTEXITCODE)." }
    & xmake build -y -v
    if ($LASTEXITCODE -ne 0) { throw "xmake build failed ($LASTEXITCODE)." }

    $BinaryPath = Join-Path $BuildDirectory 'windows/x64/release/P3RFix.asi'
    if (-not (Test-Path -LiteralPath $BinaryPath -PathType Leaf)) {
        throw "The release binary was not produced: $BinaryPath"
    }

    $LoaderZipPath = Join-Path $DownloadDirectory $LoaderArchive
    if (-not (Test-Path -LiteralPath $LoaderZipPath -PathType Leaf)) {
        $LoaderUrl = "https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases/download/$LoaderVersion/$LoaderArchive"
        Invoke-WebRequest -Uri $LoaderUrl -OutFile $LoaderZipPath
    }
    if ((Get-FileHash -LiteralPath $LoaderZipPath -Algorithm SHA256).Hash -ne $LoaderSha256) {
        throw 'Ultimate ASI Loader archive failed SHA256 verification.'
    }
    Expand-Archive -LiteralPath $LoaderZipPath -DestinationPath $LoaderDirectory
    $LoaderDllPath = Join-Path $LoaderDirectory 'dinput8.dll'
    if (-not (Test-Path -LiteralPath $LoaderDllPath -PathType Leaf)) {
        throw 'Ultimate ASI Loader archive does not contain dinput8.dll.'
    }

    foreach ($PackageDirectory in @($StandaloneDirectory, $ReloadedDirectory)) {
        Copy-Item -LiteralPath $BinaryPath, 'P3RFix.ini', $CombinedLicensePath `
            -Destination $PackageDirectory
    }
    Copy-Item -LiteralPath $LoaderDllPath -Destination (Join-Path $StandaloneDirectory 'dsound.dll')

    $ModConfig = Get-Content -LiteralPath 'assets/r2-package/ModConfig.json' -Raw | ConvertFrom-Json
    $ModConfig.ModVersion = $Version
    $ModConfig | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath (Join-Path $ReloadedDirectory 'ModConfig.json') -Encoding utf8NoBOM

    Compress-Archive -Path (Join-Path $StandaloneDirectory '*') -DestinationPath $ZipPath
    Compress-Archive -Path (Join-Path $ReloadedDirectory '*') -DestinationPath $ReloadedZipPath

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $ReferenceLicenses = [System.IO.File]::ReadAllBytes($CombinedLicensePath)
    $StandaloneLicenses = Assert-PackageContent -ArchivePath $ZipPath `
        -ReferenceLicenses $ReferenceLicenses -Standalone $true
    $ReloadedLicenses = Assert-PackageContent -ArchivePath $ReloadedZipPath `
        -ReferenceLicenses $ReferenceLicenses -Standalone $false
    if (-not (Test-ByteArraysEqual -First $StandaloneLicenses -Second $ReloadedLicenses)) {
        throw 'Packaged LICENSES documents do not match each other.'
    }
    Write-Information 'Both packaged LICENSES documents match the generated reference.' -InformationAction Continue

    $ReleaseBody = (Get-Content -LiteralPath 'release_body.md' -Raw).
        Replace('<RELEASE_ZIP_NAME>', $ZipName).Replace('<VERSION>', $Version)
    if (Test-Path -LiteralPath 'CHANGELOG.md') {
        $Changelog = Get-Content -LiteralPath 'CHANGELOG.md' -Raw
        $EscapedVersion = [regex]::Escape($Version)
        $Changes = [regex]::Match($Changelog,
            "(?ms)^## \[$EscapedVersion\][^\r\n]*\r?\n(?<body>.*?)(?=^## |\z)")
        if ($Changes.Success) {
            $ReleaseBody = "## Changes`n`n$($Changes.Groups['body'].Value.Trim())`n`n$ReleaseBody"
        }
    }
    $ReleaseBody | Set-Content -LiteralPath $ReleaseBodyPath -Encoding utf8NoBOM

    Remove-Item -LiteralPath $StagingDirectory -Recurse -Force
    Write-Information "Created $ZipPath and $ReloadedZipPath" -InformationAction Continue
} finally {
    Pop-Location
}
