const functions = require("firebase-functions");
const { BigQuery } = require('@google-cloud/bigquery');
const admin = require("firebase-admin");
admin.initializeApp();
const bigqueryClient = new BigQuery();

exports.sendSensorDataToBigQuery = functions.database
  .ref('/mac_addresses/{pushId}')
  .onCreate(async (snapshot, context) => {
         

    const data = snapshot.val();
    const { macAddress, timestamp } = data;
   
    try {
      // Save data to BigQuery
      const dataset = bigqueryClient.dataset('MAC_Addresses');
      const table = dataset.table('Mac_Addresses_Table');

      const rows = [{ macAddress: macAddress, timestamp:timestamp }];
      await table.insert(rows);
    } catch (error) {
      console.error(error);
    }
  });
// // Create and deploy your first functions
// // https://firebase.google.com/docs/functions/get-started
//
// exports.helloWorld = functions.https.onRequest((request, response) => {
//   functions.logger.info("Hello logs!", {structuredData: true});
//   response.send("Hello from Firebase!");
// });

