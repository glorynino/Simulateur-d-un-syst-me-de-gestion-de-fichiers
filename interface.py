import tkinter as tk
from tkinter import ttk, messagebox
import random
from ttkbootstrap import Style
import time


FACTEUR_BLOCAGE = 5
NB_BLOCS = 20

class SelectionPage:
    def __init__(self):
        self.root = tk.Tk()
        self.style = Style(theme='cosmo')
        self.root.title("Sélection du type d'organisation")
        self.root.geometry("400x300")
        self.root.configure(bg='#f0f5f9')
        
        # Frame centrale
        main_frame = ttk.Frame(self.root)
        main_frame.pack(expand=True)
        
        # Titre
        title_label = ttk.Label(
            main_frame,
            text="Choisissez le type d'organisation",
            font=('Helvetica', 16, 'bold'),
            foreground='#1e3d59'
        )
        title_label.pack(pady=20)
        
        # Boutons
        ttk.Button(
            main_frame,
            text="Organisation Contiguë",
            command=self.lancer_contigue,
            style='success.TButton'
        ).pack(pady=10)
        
        ttk.Button(
            main_frame,
            text="Organisation Chaînée",
            command=self.lancer_chainee,
            style='info.TButton'
        ).pack(pady=10)
        
    def lancer_contigue(self):
        self.root.destroy()
        app = MemoryManagerUI(mode="contigue")
        app.run()
        
    def lancer_chainee(self):
        self.root.destroy()
        app = MemoryManagerUI(mode="chainee")
        app.run()
        
    def run(self):
        self.root.mainloop()

class Bloc:
    def __init__(self):
        self.nom_fichier = None
        self.nb_enregistrements = 0
        self.libre = True
        self.suivant = None  # Pour le chaînage

    def __str__(self):
        if self.libre:
            return "Bloc libre"
        return f"{self.nom_fichier} - {self.nb_enregistrements} enregistrements"
    
class MemoryManagerUI:
    def __init__(self, mode="contigue"):
        self.mode = mode
        self.root = tk.Tk()
        self.style = Style(theme='cosmo')
        
        self.root.title("Gestionnaire de Mémoire Secondaire")
        self.root.geometry("1000x800")  # Hauteur augmentée
        self.root.configure(bg='#f0f5f9')
        
        self.setup_variables()
        self.create_widgets()
        self.MS = [Bloc() for _ in range(NB_BLOCS)]
        self.afficher_etat_ms()
        
    def setup_variables(self):
        self.nom_var = tk.StringVar()
        self.nb_enregistrements_var = tk.StringVar()
        
    def create_widgets(self):
        # Frame principale
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill='both', expand=True, padx=20, pady=10)
        
        # Titre
        title_label = ttk.Label(
            main_frame,
            text="Gestionnaire de Mémoire Secondaire",
            font=('Helvetica', 24, 'bold'),
            foreground='#1e3d59'
        )
        title_label.pack(pady=10)

        # Canvas pour la visualisation - réduit en hauteur
        self.canvas = tk.Canvas(
            main_frame,
            width=800,
            height=350,
            bg='#ffffff',
            highlightthickness=0
        )
        self.canvas.pack(pady=10)

        # Frame pour les contrôles
        control_frame = ttk.Frame(main_frame)
        control_frame.pack(fill='x', pady=10)

        # Inputs
        input_frame = ttk.LabelFrame(
            control_frame,
            text="Paramètres",
            padding=15
        )
        input_frame.pack(padx=20, pady=5, fill='x')

        # Grille d'entrées
        ttk.Label(input_frame, text="Nom du fichier:").grid(row=0, column=0, padx=5, pady=5)
        nom_entry = ttk.Entry(input_frame, textvariable=self.nom_var)
        nom_entry.grid(row=0, column=1, padx=5, pady=5)

        ttk.Label(input_frame, text="Nombre d'enregistrements:").grid(row=1, column=0, padx=5, pady=5)
        nb_entry = ttk.Entry(input_frame, textvariable=self.nb_enregistrements_var)
        nb_entry.grid(row=1, column=1, padx=5, pady=5)

        # Boutons
        button_frame = ttk.Frame(control_frame)
        button_frame.pack(pady=10)

        buttons = [
            ("Créer Fichier", self.creer_fichier, 'success'),
            ("Supprimer Fichier", self.supprimer_fichier, 'danger'),
            ("Rechercher", self.rechercher_enregistrement, 'info'),
            ("Insérer Enreg.", self.inserer_enregistrement, 'primary'),
            ("Défragmenter", self.defragmenter_fichier, 'warning'),
            ("Compactage", self.compacter, 'secondary'),
            ("Réinitialiser", self.reinitialiser, 'danger')
        ]

        for i, (text, command, style) in enumerate(buttons):
            btn = ttk.Button(
                button_frame,
                text=text,
                command=command,
                style=f'{style}.TButton'
            )
            btn.grid(row=i//4, column=i%4, padx=5, pady=5)

        # Zone de métadonnées - réduite en hauteur
        self.text_metadonnees = tk.Text(
            main_frame,
            width=80,
            height=8,
            font=('Consolas', 10),
            bg='#ffffff',
            wrap=tk.WORD
        )
        self.text_metadonnees.pack(pady=10)

    def afficher_etat_ms(self):
        self.canvas.delete("all")
        
        # Dimensions réduites des blocs
        largeur_bloc = 140  # Réduit
        hauteur_bloc = 50   # Réduit
        x_offset, y_offset = 30, 20  # Ajusté
        espacement_h = 15   # Réduit
        espacement_v = 15   # Réduit
        blocs_par_ligne = 4
        
        for i, bloc in enumerate(self.MS):
            x1 = x_offset + (i % blocs_par_ligne) * (largeur_bloc + espacement_h)
            y1 = y_offset + (i // blocs_par_ligne) * (hauteur_bloc + espacement_v)
            x2 = x1 + largeur_bloc
            y2 = y1 + hauteur_bloc
            
            # Effet d'ombre
            self.canvas.create_rectangle(
                x1+2, y1+2, x2+2, y2+2,
                fill='#dddddd',
                outline='#dddddd'
            )
            
            # Bloc principal
            color = '#4CAF50' if bloc.libre else '#F44336'
            self.canvas.create_rectangle(
                x1, y1, x2, y2,
                fill=color,
                outline='white',
                width=2
            )
            
            # Texte du bloc
            texte_bloc = f"Bloc {i+1}\nLibre" if bloc.libre else f"{bloc.nom_fichier}\n{bloc.nb_enregistrements}"
            text_color = 'white'
            
            # Ombre du texte
            self.canvas.create_text(
                (x1 + x2) // 2 + 1,
                (y1 + y2) // 2 + 1,
                text=texte_bloc,
                fill='black',
                font=('Arial', 10, 'bold'),
                justify='center'
            )
            
            # Texte principal
            self.canvas.create_text(
                (x1 + x2) // 2,
                (y1 + y2) // 2,
                text=texte_bloc,
                fill=text_color,
                font=('Arial', 10, 'bold'),
                justify='center'
            )

    def afficher_animation(self, bloc_index):
        blocs_par_ligne = 4
        largeur_bloc = 140
        hauteur_bloc = 50
        espacement_h = 15
        espacement_v = 15
        x_offset, y_offset = 30, 20
        
        x1 = x_offset + (bloc_index % blocs_par_ligne) * (largeur_bloc + espacement_h)
        y1 = y_offset + (bloc_index // blocs_par_ligne) * (hauteur_bloc + espacement_v)
        
        for i in range(5):
            self.canvas.create_oval(
                x1-i*2, y1-i*2,
                x1+largeur_bloc+i*2, y1+hauteur_bloc+i*2,
                outline='#FFD700',
                width=2
            )
            self.root.update()
            time.sleep(0.05)
            self.canvas.delete("all")
            self.afficher_etat_ms()

    def afficher_metadonnees(self):
        self.text_metadonnees.delete(1.0, tk.END)
        fichiers = {}
        for i, bloc in enumerate(self.MS):
            if not bloc.libre:
                if bloc.nom_fichier not in fichiers:
                    fichiers[bloc.nom_fichier] = []
                fichiers[bloc.nom_fichier].append(i)
        
        for nom, blocs in fichiers.items():
            self.text_metadonnees.insert(tk.END, f"📁 {nom} : Blocs → {blocs}\n")

    # Les méthodes de gestion restent les mêmes mais sont adaptées à la classe
    def creer_fichier(self):
        nom = self.nom_var.get()
        try:
            nb_enregistrements = int(self.nb_enregistrements_var.get())
        except ValueError:
            messagebox.showerror("Erreur", "Le nombre d'enregistrements doit être un entier.")
            return

        blocs_necessaires = (nb_enregistrements + FACTEUR_BLOCAGE - 1) // FACTEUR_BLOCAGE
        blocs_libres = [i for i, bloc in enumerate(self.MS) if bloc.libre]
        
        if len(blocs_libres) < blocs_necessaires:
            messagebox.showerror("Erreur", "Pas assez d'espace libre pour créer le fichier.")
            return

        if self.mode == "chainee":
            random.shuffle(blocs_libres)
            blocs_a_utiliser = blocs_libres[:blocs_necessaires]
            for i in range(len(blocs_a_utiliser) - 1):
                self.MS[blocs_a_utiliser[i]].suivant = blocs_a_utiliser[i + 1]
        
        for i in blocs_libres[:blocs_necessaires]:
            self.MS[i].nom_fichier = nom
            self.MS[i].nb_enregistrements = min(nb_enregistrements, FACTEUR_BLOCAGE)
            self.MS[i].libre = False
            nb_enregistrements -= FACTEUR_BLOCAGE
            self.afficher_animation(i)

        self.afficher_etat_ms()
        self.afficher_metadonnees()
        messagebox.showinfo("Succès", f"Le fichier '{nom}' a été créé avec succès.")

    def supprimer_fichier(self):
        nom = self.nom_var.get()
        trouve = False
        for bloc in self.MS:
            if not bloc.libre and bloc.nom_fichier == nom:
                bloc.nom_fichier = None
                bloc.nb_enregistrements = 0
                bloc.libre = True
                trouve = True

        if trouve:
            self.afficher_etat_ms()
            self.afficher_metadonnees()
            messagebox.showinfo("Succès", f"Le fichier '{nom}' a été supprimé.")
        else:
            messagebox.showerror("Erreur", "Fichier non trouvé.")

    def rechercher_enregistrement(self):
        nom = self.nom_var.get()
        try:
            enregistrement_id = int(self.nb_enregistrements_var.get())
        except ValueError:
            messagebox.showerror("Erreur", "L'ID de l'enregistrement doit être un entier.")
            return

        for i, bloc in enumerate(self.MS):
            if bloc.nom_fichier == nom and 0 < enregistrement_id <= bloc.nb_enregistrements:
                self.afficher_animation(i)
                messagebox.showinfo("Succès", f"Enregistrement trouvé dans le bloc {i}.")
                return

        messagebox.showerror("Erreur", "Enregistrement non trouvé.")

    def inserer_enregistrement(self):
        nom = self.nom_var.get()
        for i, bloc in enumerate(self.MS):
            if bloc.nom_fichier == nom and bloc.nb_enregistrements < FACTEUR_BLOCAGE:
                bloc.nb_enregistrements += 1
                self.afficher_animation(i)
                self.afficher_etat_ms()
                self.afficher_metadonnees()
                messagebox.showinfo("Succès", "Enregistrement inséré avec succès.")
                return

        messagebox.showerror("Erreur", "Impossible d'insérer un enregistrement.")

    def defragmenter_fichier(self):
        nom = self.nom_var.get()
        fichiers = [bloc for bloc in self.MS if bloc.nom_fichier == nom]
        if not fichiers:
            messagebox.showerror("Erreur", "Fichier non trouvé.")
            return

        blocs_libres = [bloc for bloc in self.MS if bloc.libre]
        if len(blocs_libres) < len(fichiers):
            messagebox.showerror("Erreur", "Pas assez d'espace libre pour défragmenter.")
            return

        self.MS.sort(key=lambda b: (b.libre, b.nom_fichier != nom))
        self.afficher_etat_ms()
        self.afficher_metadonnees()
        messagebox.showinfo("Succès", f"Le fichier '{nom}' a été défragmenté.")

    def reinitialiser(self):
        self.MS = [Bloc() for _ in range(NB_BLOCS)]
        self.afficher_etat_ms()
        self.afficher_metadonnees()

    def compacter(self):
        blocs_occupees = [bloc for bloc in self.MS if not bloc.libre]
        blocs_libres = [Bloc() for _ in range(len(self.MS) - len(blocs_occupees))]
        self.MS = blocs_occupees + blocs_libres
        self.afficher_etat_ms()
        self.afficher_metadonnees()

    def run(self):
        self.root.mainloop()

if __name__ == "__main__":
    selection = SelectionPage()
    selection.run()