# Ouvrir PowerShell en tant qu'admin puis:
#
# uninstall
# .\setup_task.ps1 -Uninstall
#
# install
# .\setup_task.ps1
#
# install and run w/o reboot
# .\setup_task.ps1 -RunNow


<#
  setup_midi2artnet_win.ps1
  - auto relaunch with ExecutionPolicy Bypass & no window if necessary
  - creates a scheduled task that launches the project's venv python at startup
  - options:
      -Broadcast   : adresse de broadcast (ex: 192.168.1.255)
      -Universe    : numéro d’univers ArtNet (par défaut 0)
      -RunNow      : start the scheduled task immediately after creation
      -Uninstall   : remove the scheduled task
#>

param(
    [string]$TaskName = "MIDI2ArtNet",
    [string]$Broadcast = "192.168.1.255",
    [int]$Universe = 0,
    [switch]$Uninstall,
    [switch]$RunNow
)

# -------------------------
# Auto relaunch with Bypass
# -------------------------
if (-not $env:MIDI2ARTNET_BYPASS) {
    $scriptPath = $PSCommandPath
    if (-not $scriptPath) { $scriptPath = $MyInvocation.MyCommand.Definition }

    Write-Host "Relance du script en mode Bypass (exécution silencieuse)..."

    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = (Get-Command powershell.exe).Source
    $psi.Arguments = "-NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File `"$scriptPath`" @args"
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.EnvironmentVariables["MIDI2ARTNET_BYPASS"] = "1"

    try {
        [System.Diagnostics.Process]::Start($psi) | Out-Null
    } catch {
        Write-Warning "Impossible de relancer le script en Bypass automatiquement. Lance manuellement :"
        Write-Warning "powershell -ExecutionPolicy Bypass -File `"$scriptPath`""
    }
    exit
}

# -------------------------
# Main logic
# -------------------------
$ProjectPath = (Get-Location).Path

if ($Uninstall) {
    if (Get-ScheduledTask -TaskName $TaskName -ErrorAction SilentlyContinue) {
        Unregister-ScheduledTask -TaskName $TaskName -Confirm:$false
        Write-Host "❌ Tâche planifiée '$TaskName' supprimée."
    } else {
        Write-Host "⚠️ Aucune tâche nommée '$TaskName' trouvée."
    }
    exit
}

$venvPath = Join-Path $ProjectPath "venv"
$pythonExe = Join-Path $venvPath "Scripts\python.exe"
$scriptPath = Join-Path $ProjectPath "midi2artnet.py"

# Crée le venv si absent et installe dépendances
if (-not (Test-Path $pythonExe)) {
    Write-Host "📦 Création de l'environnement virtuel dans $venvPath ..."
    Push-Location $ProjectPath
    python -m venv venv

    if (-not (Test-Path $pythonExe)) {
        Write-Error "Échec : python.exe introuvable dans le venv."
        Pop-Location
        exit 1
    }

    & $pythonExe -m pip install --upgrade pip
    & $pythonExe -m pip install mido python-rtmidi
    Pop-Location
}

if (-not (Test-Path $scriptPath)) {
    Write-Error "Script python introuvable : $scriptPath."
    exit 1
}

# Action avec paramètres dynamiques
$action = New-ScheduledTaskAction -Execute $pythonExe `
          -Argument "`"$scriptPath`" --broadcast $Broadcast --universe $Universe"

$trigger = New-ScheduledTaskTrigger -AtStartup
$settings = New-ScheduledTaskSettingsSet -AllowStartIfOnBatteries -DontStopIfGoingOnBatteries -Hidden
$principal = New-ScheduledTaskPrincipal -UserId "$env:UserName" -RunLevel Highest

Register-ScheduledTask -TaskName $TaskName -Action $action -Trigger $trigger -Settings $settings -Principal $principal -Force

Write-Host "✅ Tâche planifiée '$TaskName' créée."
Write-Host "Broadcast = $Broadcast , Universe = $Universe"
Write-Host "Le script $scriptPath sera exécuté au démarrage (silent)."

if ($RunNow) {
    Start-ScheduledTask -TaskName $TaskName
    Write-Host "▶️ Tâche '$TaskName' démarrée (RunNow)."
}
