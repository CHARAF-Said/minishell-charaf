# MiniShell - Projet C

Ce projet est un mini-shell développé en C sous Ubuntu WSL.

## Fonctions disponibles

- cd, exit, env (commandes internes)
- ls, cat, echo (commandes externes)
- Redirection > < >>
- Pipes |
- Exécution en arrière-plan avec &

## Compilation

```bash
gcc main.c -o minishell
./minishell
