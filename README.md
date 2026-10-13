# Controlcenter SIGNALCONTROL

Programma separato di monitoraggio. Non comanda lampade o centraline. Parte in modalità simulata, senza hardware.

## Avvio

Apri index.html per usare la simulazione. Per USB, dalla cartella del monitor esegui:

```sh
python3 -m http.server 8081
```

Apri http://localhost:8081 in Chrome o Edge desktop. Seleziona ESP32 via USB e premi Collega ESP32. Il browser chiede di scegliere la porta. Velocità: 115200 baud. Nessun comando viene inviato al dispositivo. L'apertura della porta può riavviare alcune schede ESP32.

## Prove

- Seleziona A rosso / B rosso e attiva Guasto rosso A: appare l'anomalia.
- Prendi in carico: il guasto rimane attivo.
- Disattiva Guasto rosso A: appare l'evento di ripristino.
- Disattiva il riscontro: non viene dichiarata l'effettiva accensione delle lampade.
- Interrompi comunicazione: dopo 3,5 secondi lo stato diventa sconosciuto.
- Esporta CSV per scaricare il registro della sessione (massimo 500 eventi).

## Protocollo ESP32

Una riga JSON UTF-8 per messaggio, terminata da newline, circa ogni secondo:

```json
{"type":"status","version":1,"boot_id":"boot-123","seq":1,"signals":[{"id":"A","command":"red","feedback":null},{"id":"B","command":"red","feedback":null}],"faults":[]}
```

command: red / yellow / green / off. feedback: stessi valori oppure null se non disponibile. Non copiare il comando nel riscontro per simulare una misura reale.

boot_id cambia ad ogni riavvio, seq cresce ad ogni messaggio. Messaggi fuori ordine dello stesso boot vengono ignorati. Sono richiesti i segnali A e B. faults contiene tutti i guasti attualmente attivi, per esempio:

```json
[{"code":"RED_A_FAILURE","message":"Semaforo A: guasto luce rossa"}]
```

La rimozione di un guasto da un messaggio valido indica il ripristino. Il monitor rileva anche discrepanze fra comando e riscontro. Soglie, temporizzazioni e diagnosi elettriche vanno implementate e verificate nel firmware/hardware.

## Limiti

Telemetria USB verificata su ESP32-D0WD-V3 tramite servizio locale. Il firmware originale non è disponibile in questo progetto: occorre aggiungere l'emissione dei messaggi senza alterare la logica delle fasi. Nessun sensore installato, nessuna diagnostica reale disponibile. Il registro non è persistente. La reazione ai guasti dell'impianto è responsabilità del controllo di sicurezza, non di questo monitor sperimentale.

## Esempio firmware

esp32-telemetry-demo.ino emette messaggi di prova compatibili via seriale: comando rosso per A e B e riscontro assente. Non pilota pin o lampade e non include la logica KI Traffic. Non sostituire il firmware originale senza averne salvato il progetto. Esempio compilato e caricato con successo sulla scheda ESP32 collegata; messaggi USB verificati.

Una discrepanza già rilevata resta attiva se il riscontro diventa indisponibile: l'assenza di misura non prova un ripristino.

## Collegamento locale USB (anche nel browser integrato)

Il file usb-monitor.py legge l'ESP32 e serve il monitor sullo stesso indirizzo. Richiede pyserial. Avvio:

```sh
python3 usb-monitor.py --port /dev/cu.usbserial-120
```

Apri http://localhost:8082 e scegli ESP32 USB tramite collegamento locale. La porta può essere aperta da un solo programma alla volta. Il servizio è limitato a questo computer.

## Copia di ripristino della scheda

La copia locale esp32-backup-before-upload.bin (esclusa dal repository pubblico) conserva i primi 0x60000 byte della scheda collegata prima dell'installazione. È una copia di tutti i settori modificati da questo caricamento, non dell'intera memoria da 8 MB. Il resto della memoria non è stato cancellato. Per ripristinare, finché il resto della flash è invariato, usare esptool write-flash con indirizzo 0 e questa copia. Non caricare il backup su altre schede o repository pubblici.

## Ciclo LED da banco

esp32-led-cycle.ino comanda verde (GPIO27), giallo (GPIO26), rosso (GPIO25), per 3 secondi ciascuno, continuamente. Collegamenti confermati dall'utente: ciascun LED tramite resistenza 220 ohm e ritorno a GND. Telemetria ogni 500 ms e ad ogni cambio; segnale A rappresenta i tre mini LED, segnale B è spento/non collegato. Riscontro null: non è una misura fisica della luce. Questa sequenza è una prova da banco, non una sequenza per traffico stradale.

## Interfaccia operatore

Stile industriale e selettore IT/DE. La lingua viene memorizzata nel browser. Interfaccia, anomalie interne, registro e intestazioni CSV sono tradotti. I messaggi liberi forniti da firmware esterni restano nella lingua della sorgente quando non esiste una traduzione nota.

Il servizio USB si riconnette automaticamente alla stessa porta dopo un distacco. Durante l’assenza della scheda i dati restano sconosciuti. Se il sistema assegna una porta diversa, riavviare il servizio con la nuova porta.
