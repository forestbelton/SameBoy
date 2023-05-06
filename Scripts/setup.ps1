$BASE_DIR = "C:\decompile"
$DOWNLOAD_DIR = "$BASE_DIR\cache"

$DEPENDENCIES = @{
    SAMEBOY = @{
        URL = "https://github.com/forestbelton/SameBoy/archive/refs/heads/decompile.zip";
        Expand = $true;
    };
    SDL2 = @{
        URL = "https://github.com/libsdl-org/SDL/releases/download/release-2.26.5/SDL2-devel-2.26.5-VC.zip";
        Expand = $true;
    };
    RGBDS = @{
        URL = "https://github.com/gbdev/rgbds/releases/download/v0.6.1/rgbds-0.6.1-win64.zip";
    }
}

New-Item -ItemType Directory -Path $DOWNLOAD_DIR -Force | Out-Null
$httpClient = New-Object System.Net.WebClient
foreach ($dependency in $DEPENDENCIES.Keys) {
    $dir = "$BASE_DIR\$dependency"
    $zip = "$DOWNLOAD_DIR\${dependency}.zip"
    $info = $DEPENDENCIES[$dependency];

    if (-not(Test-Path -Path $zip -PathType Leaf)) {
        $httpClient.DownloadFile($info.URL, $zip)
    }

    if (Test-Path -Path $dir -PathType Container) {
        Get-ChildItem -Path $dir -Recurse | Remove-Item -Force -Recurse
        Remove-Item $dir -Force
    }

    New-Item -ItemType Directory -Path $dir -Force | Out-Null
    Expand-Archive -LiteralPath $zip -DestinationPath $dir

    if ($info.Expand) {
        $nestedDir = (Get-ChildItem -Path $dir -Directory).FullName
        Get-ChildItem -Path "$dir\*" -Recurse | Move-Item -Destination $dir
        Remove-Item -Path $nestedDir
    }
}
