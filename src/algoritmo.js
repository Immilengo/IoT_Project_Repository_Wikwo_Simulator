const functions = require("firebase-functions");
const admin = require("firebase-admin");

admin.initializeApp();

const PRESSAO_MIN = 0.5; // bar
const JANELA_DIAS = 7;

exports.processarLeitura = functions.database
  .ref("/botijas/{botijaId}/leiturasRaw/{leituraId}")
  .onCreate(async (snap, context) => {

    const leitura = snap.val();
    const botijaId = context.params.botijaId;
    const db = admin.database();

    const pressao = leitura.pressao;
    const temperatura = leitura.temperatura;

    // ---------- VALIDACAO ----------
    if (pressao < 0 || pressao > 8) return null;
    if (temperatura < -10 || temperatura > 60) return null;

    const agora = Date.now();
    const data = new Date(agora).toISOString().split("T")[0];

    // ---------- ESTADO ATUAL ----------
    await db.ref(`/botijas/${botijaId}/estadoAtual`).set({
      pressaoAtual: pressao,
      temperaturaAtual: temperatura,
      atualizadoEm: agora
    });

    // ---------- AGREGAÇÃO DIÁRIA ----------
    const diaRef = db.ref(`/botijas/${botijaId}/dias/${data}`);
    const diaSnap = await diaRef.get();

    let somaPressao = pressao;
    let somaTemp = temperatura;
    let contador = 1;

    if (diaSnap.exists()) {
      const d = diaSnap.val();
      somaPressao += d.somaPressao;
      somaTemp += d.somaTemperatura;
      contador += d.contador;
    }

    await diaRef.set({
      somaPressao,
      somaTemperatura: somaTemp,
      contador,
      pressaoMedia: somaPressao / contador,
      temperaturaMedia: somaTemp / contador
    });

    // ---------- CONSUMO DIÁRIO ----------
    const diasSnap = await db
      .ref(`/botijas/${botijaId}/dias`)
      .orderByKey()
      .limitToLast(2)
      .get();

    if (diasSnap.numChildren() < 2) return null;

    const dias = Object.values(diasSnap.val());
    const consumo = dias[0].pressaoMedia - dias[1].pressaoMedia;

    if (consumo <= 0 || consumo > 2) return null;

    await diaRef.update({ consumo });

    // ---------- CONSUMO MÉDIO ----------
    const ultimosDiasSnap = await db
      .ref(`/botijas/${botijaId}/dias`)
      .orderByKey()
      .limitToLast(JANELA_DIAS)
      .get();

    let somaConsumo = 0;
    let diasValidos = 0;

    ultimosDiasSnap.forEach(d => {
      if (d.val().consumo) {
        somaConsumo += d.val().consumo;
        diasValidos++;
      }
    });

    if (diasValidos < 3) return null;

    const consumoMedio = somaConsumo / diasValidos;

    // ---------- PREVISAO ----------
    const pressaoUtil = pressao - PRESSAO_MIN;
    const diasRestantes = Math.floor(pressaoUtil / consumoMedio);

    let nivel = "NORMAL";
    if (diasRestantes <= 10) nivel = "AVISO";
    if (diasRestantes <= 3) nivel = "CRITICO";

    await db.ref(`/botijas/${botijaId}/previsao`).set({
      consumoMedioDiario: consumoMedio,
      diasRestantesEstimados: diasRestantes,
      nivelRisco: nivel,
      atualizadoEm: agora
    });

    return null;
  });
