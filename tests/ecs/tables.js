const azure = require('azure-storage');
const util = require('util');
const http = require("http");

const azhost = "https://ariasdkbvt.table.core.windows.net";
const tableSAS = "?sv=2017-04-17&si=ExpConfigDemo-16225D8E5EA&tn=expconfigdemo&sig=SMKnjjUvukLB%2F6Ea301pf7MZjsO5N2mjIOCpN%2FxwT5M%3D";

const sharedTableService = azure.createTableServiceWithSas(azhost, tableSAS);
const query = azure.TableQuery();
const table = "ExpConfigDemo";

const createLogger = require("logging");
const logger = createLogger.default("CfgProvider");

const port  = 5002;

/**
 * Async function to retrieve AzureDB entries as array of objects
 */
function getConfig() {
	return new Promise((resolve, reject) => {
		sharedTableService.queryEntities(table, query, null, function (error, result, response) {
			var out = Array();
			if (!error) {
				logger.info("Result:", result);
				result["entries"].forEach(function (item) {
					out.push({
						"EventName": item["EventName"]["_"],
						"SamplingRate": item["SamplingRate"]["_"]
					});
				});
				logger.info("AzureDB entries retrieved", out);
				resolve(out);
			} else {
				logger.error("AzureDB error", response);
				reject(response);
			}
		});
	});
}

/**
 * Create an instance of simple HTTP server
 */
var server = http.createServer(function (request, response) {

		// Get configuration
		getConfig()

		// If successful, return 200 and JSON config
		.then(
			(body) => {
			response.writeHead(200, {
				'Content-Type': 'application/json',
				'Cache-Control': 'no-cache'
			});
			response.write(JSON.stringify(body));
			response.end();
		})

		// If not, return 500 and error status code
		.catch(
			(error) => {
			response.writeHead(500, {
				'Content-Type': 'application/json',
				'Cache-Control': 'no-cache'
			});
			response.write(error);
			response.end();
		});

	}).listen(port);

logger.info("HTTP server started on port 5002");
logger.info(server);
