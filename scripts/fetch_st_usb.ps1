<#
.SYNOPSIS
    获取 STM32 USB Device Library（SLA0044），本仓库不随源码分发。

.DESCRIPTION
    该库适用 ST 的 SLA0044（Ultimate Liberty）：
      - 第 4 条：只能与 ST 制造的 MCU/MPU 配合使用；
      - 第 5 条：不得以任何方式使其受任何开源许可（GPL/EPL/Apache/BSD/MIT 等）约束。
    本项目固件以 GPL-3.0-only 发布，二者不兼容，因此该库不进入本仓库，
    由使用者自行从 ST 官方渠道获取。脚本只写入被 .gitignore 排除的目录。

.PARAMETER CubeRoot
    本机 STM32CubeF4 包根目录（其下应有 Middlewares/ST/STM32_USB_Device_Library）。

.PARAMETER Download
    从 https://github.com/STMicroelectronics/STM32CubeF4 稀疏克隆后复制。

.EXAMPLE
    .\fetch_st_usb.ps1 -CubeRoot C:\STM32Cube\STM32Cube_FW_F4_V1.28.0

.EXAMPLE
    .\fetch_st_usb.ps1 -Download
#>
[CmdletBinding(DefaultParameterSetName = 'Local')]
param(
    [Parameter(ParameterSetName = 'Local', Mandatory = $true)]
    [string]$CubeRoot,

    [Parameter(ParameterSetName = 'Download', Mandatory = $true)]
    [switch]$Download
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$relPath  = 'Middlewares/ST/STM32_USB_Device_Library'
$targets  = @(
    (Join-Path $repoRoot 'firmware/MCU/Application/Middlewares/ST/STM32_USB_Device_Library'),
    (Join-Path $repoRoot 'firmware/MCU/SCPI_Test/Middlewares/ST/STM32_USB_Device_Library')
)

$tempDir = $null
try {
    if ($Download) {
        $tempDir = Join-Path ([IO.Path]::GetTempPath()) ('stm32cubef4-' + [guid]::NewGuid().ToString('N'))
        New-Item -ItemType Directory -Path $tempDir | Out-Null
        Write-Host "正在稀疏克隆 STM32CubeF4 ..." -ForegroundColor Cyan
        git clone --depth 1 --filter=blob:none --sparse `
            https://github.com/STMicroelectronics/STM32CubeF4.git $tempDir
        git -C $tempDir sparse-checkout set $relPath
        $source = Join-Path $tempDir $relPath
    }
    else {
        $source = Join-Path $CubeRoot $relPath
        if (-not (Test-Path -LiteralPath $source)) {
            throw "未找到 $source。请确认 -CubeRoot 指向 STM32CubeF4 包根目录（其下应有 $relPath）。"
        }
    }

    foreach ($target in $targets) {
        if (Test-Path -LiteralPath $target) {
            Write-Host "已存在，跳过：$target" -ForegroundColor DarkGray
            continue
        }
        $parent = Split-Path -Parent $target
        New-Item -ItemType Directory -Path $parent -Force | Out-Null
        Copy-Item -LiteralPath $source -Destination $target -Recurse -Force
        Write-Host "已复制：$target" -ForegroundColor Green
    }
}
finally {
    if ($tempDir -and (Test-Path -LiteralPath $tempDir)) {
        Remove-Item -LiteralPath $tempDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

Write-Host ""
Write-Host "SLA0044 提示：" -ForegroundColor Yellow
Write-Host "  该库仅限 ST 器件使用，且不得置于任何开源许可之下。" -ForegroundColor Yellow
Write-Host "  它与本项目固件的 GPL-3.0 不兼容，不应提交到 Git。" -ForegroundColor Yellow
Write-Host "  后续计划迁移到 TinyUSB（MIT），见 firmware/MCU/README.md。" -ForegroundColor Yellow
