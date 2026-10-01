#Requires -Version 7.2
param (
    [ValidatePattern('^(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)$')]
    [string]$Version
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not $Version) {
    throw 'Pass a release version, for example: ./create_release.ps1 -Version 1.3.0'
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
    foreach ($OutputPath in @($ZipPath, $ReloadedZipPath, $ReleaseBodyPath)) {
        if (Test-Path -LiteralPath $OutputPath) {
            Remove-Item -LiteralPath $OutputPath -Force
        }
    }

    Write-Host "Building P3RFix $Version for Windows x64..."
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
        Copy-Item -LiteralPath $BinaryPath, 'P3RFix.ini', 'LICENSE.md',
            'THIRD_PARTY_NOTICES.md' -Destination $PackageDirectory
        Copy-Item -LiteralPath 'licenses' -Destination $PackageDirectory -Recurse
    }
    Copy-Item -LiteralPath $LoaderDllPath -Destination (Join-Path $StandaloneDirectory 'dsound.dll')

    $ModConfig = Get-Content -LiteralPath 'assets/r2-package/ModConfig.json' -Raw | ConvertFrom-Json
    $ModConfig.ModVersion = $Version
    $ModConfig | ConvertTo-Json -Depth 20 | Set-Content -LiteralPath (Join-Path $ReloadedDirectory 'ModConfig.json') -Encoding utf8NoBOM

    Compress-Archive -Path (Join-Path $StandaloneDirectory '*') -DestinationPath $ZipPath
    Compress-Archive -Path (Join-Path $ReloadedDirectory '*') -DestinationPath $ReloadedZipPath

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
    Write-Host "Created $ZipPath and $ReloadedZipPath"
} finally {
    Pop-Location
}
