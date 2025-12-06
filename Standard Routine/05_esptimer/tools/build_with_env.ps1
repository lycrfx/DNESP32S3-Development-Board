# Temporary script to set PATH, load esp-idf environment and build project
$env:Path = 'C:\Espressif\tools\tools\ninja\1.12.1;' + $env:Path
Write-Output "PATH updated"
Write-Output $env:Path

# Load ESP-IDF environment
& 'C:\Espressif\frameworks\esp-idf-v5.3.3\export.ps1'

# Show where ninja and idf.py are (diagnostics)
$ninja = & where.exe ninja 2>$null
if ($LASTEXITCODE -ne 0) { Write-Output "ninja not found" } else { Write-Output $ninja }

$idf = & where.exe idf.py 2>$null
if ($LASTEXITCODE -ne 0) { Write-Output "idf.py not found" } else { Write-Output $idf }

# Call the idf.exe directly to avoid PATH issues (use idf.py.exe if available)
if (Test-Path 'C:\Espressif\tools\idf-exe\1.0.3\idf.py.exe') {
	& 'C:\Espressif\tools\idf-exe\1.0.3\idf.py.exe' build -v
} else {
	# Fallback to python -m idf.py
	python -m idf.py build -v
}
