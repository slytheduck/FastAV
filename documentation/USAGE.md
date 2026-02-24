# Guida all'Uso di FastAV

Questa guida mostra come utilizzare FastAV attraverso esempi pratici.

## 🚀 Avvio Rapido

### Prima Esecuzione

```bash
# 1. Assicurati che clamd sia in esecuzione
sudo systemctl status clamav-daemon

# 2. Avvia FastAV
./build/fastav
```

Dovresti vedere la schermata principale con:
- Titolo "FastAV" grande e viola
- Pulsante "New Scan" centrale
- Statistiche delle scansioni precedenti
- Pulsanti per "View History" e "Update Database"

---

## 📝 Esempi d'Uso

### Esempio 1: Scansione Rapida della Home

**Scenario**: Vuoi scansionare velocemente la tua directory home.

**Passi:**
1. Clicca su **"New Scan"**
2. Nel dialogo, seleziona **"Home"** dalla lista
3. Clicca **"Start Scan"**
4. Osserva il progresso in tempo reale
5. Visualizza i risultati al termine

**Output atteso:**
```
[INFO] Scan started - 4523 files to scan
Progress: 45% (2035 / 4523 files)
Scan speed: 342.5 files/s
[COMPLETED] Scan finished in 13s
[STATS] Files: 4523, Threats: 0, Data: 2.4 GB
```

### Esempio 2: Scansione Cartella Download

**Scenario**: Hai scaricato alcuni file e vuoi verificarli.

**Passi:**
1. **"New Scan"** → Seleziona **"Downloads"**
2. FastAV scannerà automaticamente `~/Downloads`
3. Se trova minacce, verrà mostrato il **Threat Viewer**

**Se viene trovato un virus:**
```
╔════════════════════════════════════╗
║      ⚠️ Threats Detected!          ║
╚════════════════════════════════════╝

Found 1 threat(s) during the scan

┌─────────────────────────────────────────────────┐
│ Virus Name │ File Path              │ Size     │
├────────────┼────────────────────────┼──────────┤
│ EICAR-Test │ ~/Downloads/test.exe   │ 68 bytes │
└────────────┴────────────────────────┴──────────┘

⚠️ These files may be harmful.
```

### Esempio 3: Scansione Percorso Personalizzato

**Scenario**: Vuoi scansionare una cartella di progetto specifica.

**Passi:**
1. **"New Scan"**
2. Nel campo **"Custom Path"**, inserisci `/home/user/projects/webapp`
3. Oppure clicca **"Browse"** e naviga alla cartella
4. **"Start Scan"**

**Caso d'uso**: Perfetto per verificare codice sorgente scaricato o progetti clonati da GitHub.

### Esempio 4: Scansione Multipla

**Scenario**: Vuoi scansionare Documents + Downloads contemporaneamente.

**Passi:**
1. **"New Scan"**
2. Seleziona **entrambe** le opzioni:
   - ✅ Documents
   - ✅ Downloads
3. Opzionalmente aggiungi anche un percorso custom
4. **"Start Scan"**

FastAV combinerà tutti i percorsi e li scannerà in parallelo.

### Esempio 5: Visualizzare la Cronologia

**Scenario**: Vuoi vedere le scansioni passate.

**Passi:**
1. Dalla schermata principale, clicca **"View History"**
2. Vedrai una tabella con:
   - Data/ora di ogni scansione
   - Percorsi scansionati
   - Numero di file analizzati
   - Minacce trovate
   - Durata

**Esempio output:**
```
┌─────────────────────────────────────────────────────────────────┐
│ Date           │ Path        │ Files │ Data    │ Threats │ Time │
├────────────────┼─────────────┼───────┼─────────┼─────────┼──────┤
│ 2024-01-15 14:23 │ /home/user │ 12.3K │ 4.2 GB │    0    │ 2m   │
│ 2024-01-14 09:15 │ Downloads  │  523  │ 892 MB │    1    │ 15s  │
│ 2024-01-13 18:45 │ Documents  │ 2.1K  │ 1.1 GB │    0    │ 45s  │
└────────────────┴─────────────┴───────┴─────────┴─────────┴──────┘
```

### Esempio 6: Aggiornamento Database

**Scenario**: È passata una settimana e vuoi aggiornare le definizioni virus.

**Passi:**
1. Clicca **"Update Database"**
2. FastAV lancerà `freshclam` in background
3. Vedrai il progresso nella status bar:
   ```
   Downloading main.cvd...
   main.cvd updated (version: 62)
   Downloading daily.cvd...
   daily.cvd updated (version: 27123)
   Database updated successfully!
   ```

**Nota**: L'aggiornamento può richiedere 1-5 minuti a seconda della connessione.

---

## 💡 Tips & Tricks

### 🚄 Velocità Massima

**Per scansioni ultra-veloci:**

1. **Assicurati che clamd sia sempre in esecuzione**
   ```bash
   sudo systemctl enable clamav-daemon
   ```
   
2. **Evita di scansionare file di sistema** (`/usr`, `/lib`)
   - Rallentano la scansione
   - Raramente contengono virus

3. **Scansiona solo ciò che serve**
   - Home directory
   - Downloads
   - Directory di lavoro

### 📊 Interpretare le Statistiche

**Files/sec (Velocità)**:
- **< 100 files/s**: Normale su HDD o file grandi
- **100-300 files/s**: Buona velocità su SSD
- **> 300 files/s**: Eccellente! Probabilmente NVMe + piccoli file

**Minacce trovate**:
- **0**: Ottimo! Sistema pulito
- **1-2**: Probabilmente test file (EICAR) o false positives
- **> 5**: Attenzione! Possibile infezione seria

### 🔍 Cosa Fare se Trovi Minacce

1. **Non panico!** FastAV rileva, non esegue
2. **Verifica il file**:
   - È un file che conosci?
   - Potrebbe essere un falso positivo?
3. **Azioni consigliate**:
   ```bash
   # Sposta in quarantena (manualmente)
   mkdir -p ~/.fastav/quarantine
   mv /path/to/infected/file ~/.fastav/quarantine/
   
   # Oppure elimina direttamente
   rm /path/to/infected/file
   ```

### 🔐 Best Practices

**Scansioni Regolari:**
```bash
# Cron job per scansione settimanale
0 3 * * 0 /usr/local/bin/fastav --auto-scan ~/
```
*(Nota: funzionalità CLI da implementare)*

**Cosa Scansionare:**
- ✅ Downloads (sempre!)
- ✅ Email attachments
- ✅ USB drives
- ✅ Progetti scaricati
- ❌ `/usr`, `/bin` (raramente necessario)

---

## 🎮 Casi d'Uso Avanzati

### Developer Use Case

**Scenario**: Sei uno sviluppatore e cloni repository GitHub.

**Workflow:**
1. Clone repo: `git clone https://github.com/user/project`
2. Apri FastAV
3. New Scan → Custom Path → `/path/to/project`
4. Verifica prima di eseguire codice

### System Administrator Use Case

**Scenario**: Gestisci un server e vuoi scansionare file caricati dagli utenti.

**Setup:**
```bash
# Script di integrazione
#!/bin/bash
UPLOAD_DIR="/var/www/uploads"
clamdscan --stream $UPLOAD_DIR --infected --remove
```

FastAV può essere usato per verifiche puntuali via GUI.

### Security Researcher Use Case

**Scenario**: Analizzi malware samples.

**Safety First:**
```bash
# Scansiona in VM isolata
# Usa FastAV per quick triage
# Minacce trovate → Analisi approfondita
```

---

## 📱 Interfaccia Utente

### Scorciatoie Keyboard (Future Feature)

```
Ctrl+N  → New Scan
Ctrl+H  → View History
Ctrl+U  → Update Database
Ctrl+Q  → Quit
F5      → Refresh Stats
```

### Dark Theme

FastAV usa un tema scuro moderno per:
- ✅ Ridurre affaticamento occhi
- ✅ Aspetto professionale
- ✅ Migliore contrasto per lettura

**Colori Chiave:**
- 🟣 Viola (`#BB86FC`): Azioni primarie
- 🔵 Teal (`#03DAC6`): Azioni secondarie
- 🔴 Rosso (`#CF6679`): Errori/minacce
- 🟢 Verde (`#4CAF50`): Successo

---

## ⚙️ Configurazione

### Personalizzare Percorsi Comuni

Modifica `src/utils/FileScanner.cpp`:

```cpp
QStringList FileScanner::getCommonLocations() {
    QStringList locations;
    
    // Aggiungi le tue directory preferite
    locations << "/mnt/data/projects";
    locations << "/media/usb";
    locations << QDir::homePath() + "/Dropbox";
    
    return locations;
}
```

Ricompila per applicare le modifiche.

### Cambiare Tema Colori

In `src/utils/MaterialTheme.cpp`:

```cpp
// Cambia i colori Material Design
const QColor MaterialTheme::Primary = QColor("#YOUR_COLOR");
```

Colori consigliati:
- **Blue**: `#2196F3`
- **Green**: `#4CAF50`
- **Red**: `#F44336`
- **Purple**: `#BB86FC` (default)

---

## 🐛 Debugging

### Modalità Verbose

```bash
# Abilita log debug Qt
export QT_LOGGING_RULES="*.debug=true"
./fastav
```

### Controllare Connessione clamd

```bash
# Test manuale
echo "PING" | nc -U /var/run/clamav/clamd.ctl
# Output: PONG

# Se fallisce
sudo systemctl restart clamav-daemon
sudo journalctl -u clamav-daemon -f
```

---

## 📊 Report Example

Dopo una scansione completa, potresti vedere:

```
═══════════════════════════════════════
         Scan Report Summary
═══════════════════════════════════════

📁 Scanned Path: /home/user
⏱️  Duration: 2m 34s
📊 Files Scanned: 15,234
💾 Data Scanned: 8.7 GB
⚡ Average Speed: 98.7 files/s

🛡️  Threats Found: 0

✅ Your system is clean!

Last update: 2024-01-15 10:23
═══════════════════════════════════════
```

---

## 🎯 Prossimi Passi

Dopo aver familiarizzato con FastAV:

1. **Schedule regolari scansioni**: Crea cron job
2. **Integra nel workflow**: Pre-commit hooks, CI/CD
3. **Personalizza l'app**: Modifica temi, percorsi
4. **Contribuisci**: Fork su GitHub, aggiungi features

---

## ❓ FAQ Rapide

**Q: FastAV è più veloce di ClamAV GUI?**  
A: Sì! Grazie a multi-threading e clamd integration.

**Q: Posso usarlo commercialmente?**  
A: Sì, è MIT licensed. Vedi [LICENSE](LICENSE).

**Q: Supporta Windows?**  
A: Sì! Vedi [INSTALL.md](INSTALL.md) per istruzioni Windows.

**Q: Come posso contribuire?**  
A: Fork, implementa, pull request! Vedi [README.md](README.md).

**Q: FastAV elimina i virus automaticamente?**  
A: No, solo rileva. Tu decidi cosa fare.

---

**Buona scansione! 🛡️**

Per domande: [GitHub Issues](https://github.com/yourusername/fastav/issues)
