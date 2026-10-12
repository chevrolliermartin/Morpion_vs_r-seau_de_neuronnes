#!/usr/bin/env python3
import os
import sys
import subprocess
from pathlib import Path

# --- Configuration ---
# Chemin relatif vers le Doxyfile
DOXYFILE_REL_PATH = "Doxyfile"

# Récupère la racine du projet (répertoire courant lors de l'exécution)
PROJECT_ROOT = Path(os.getcwd()).absolute()
DOXYFILE_PATH = PROJECT_ROOT / DOXYFILE_REL_PATH


# --- Fonctions communes ---
def check_command_available(cmd: str) -> bool:
    """Vérifie si une commande est disponible dans le PATH."""
    try:
        subprocess.run(
            [cmd, "--version"],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=True,
        )
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False


def run_doxygen(doxyfile_path: str) -> bool:
    """Exécute Doxygen. Retourne True en cas de succès."""
    try:
        subprocess.run(
            ["doxygen", doxyfile_path],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        return True
    except subprocess.CalledProcessError as e:
        stderr = e.stderr.decode().strip() if e.stderr else str(e)
        print(f"❌ Erreur lors de l'exécution de Doxygen : {stderr}")
        return False


def generate_doc():
    """Vérifie les dépendances puis lance Doxygen si tout est disponible."""
    doxygen_ok = check_command_available("doxygen")
    graphviz_ok = check_command_available("dot")

    if not doxygen_ok or not graphviz_ok:
        print("ℹ️  Doxygen ou Graphviz n'est pas installé. La documentation ne sera pas générée.")
        print("   - Installez Doxygen : https://www.doxygen.nl/")
        print("   - Installez Graphviz : https://graphviz.org/")
        return

    print("✅ Génération de la documentation avec Doxygen...")
    if run_doxygen(str(DOXYFILE_PATH)):
        print("🎉 Documentation générée avec succès dans le dossier 'doc/html' !")
        print("ouvrir le fichier index.html")
    else:
        print("❌ Échec de la génération de la documentation.")


# --- Hook SCons (utilisé par PlatformIO via extra_scripts) ---
# Ce bloc n'est exécuté que lorsque le script est chargé par SCons
# (la fonction globale Import() n'existe que dans ce contexte).
try:
    Import("env")

    def _post_build_doxygen(source, target, env):
        print(f"📁 Cible construite : {target[0]}")
        print(f"📁 PROJECT_ROOT : {PROJECT_ROOT}")
        print(f"📁 DOXYFILE_PATH : {DOXYFILE_PATH}")
        generate_doc()

    # Accroche l'action réellement après la génération du binaire final.
    # Adapte la cible si besoin (.elf est quasi toujours généré,
    # .bin ne l'est pas forcément selon la plateforme/le framework).
    env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", _post_build_doxygen)

except NameError:
    # Pas dans un contexte SCons -> on ne fait rien ici,
    # le mode standalone est géré plus bas.
    pass


# --- Mode standalone (exécution manuelle : python scripts/run_doxygen.py) ---
if __name__ == "__main__":
    EXECUTABLE_REL_PATH = ".pio/build/nucleo_l476rg/firmware.elf"
    executable_path = Path(EXECUTABLE_REL_PATH).absolute()

    print(f"📁 PROJECT_ROOT : {PROJECT_ROOT}")
    print(f"📁 EXECUTABLE_PATH : {executable_path}")
    print(f"📁 DOXYFILE_PATH : {DOXYFILE_PATH}")

    if not executable_path.exists():
        print("⚠️  L'exécutable n'existe pas. La compilation a peut-être échoué.")
        sys.exit(0)  # Sortie silencieuse (pas d'erreur)

    generate_doc()