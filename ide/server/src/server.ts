import {
	createConnection,
	TextDocuments,
	ProposedFeatures,
	InitializeParams,
	TextDocumentSyncKind,
	InitializeResult,
} from 'vscode-languageserver/node';

import {
	TextDocument
} from 'vscode-languageserver-textdocument';

// Create a connection for the server, using Node's IPC as a transport.
// Also include all preview / proposed LSP features.
const connection = createConnection(ProposedFeatures.all);

// Create a simple text document manager.
const documents = new TextDocuments(TextDocument);

connection.onInitialize((params: InitializeParams) => {
	const capabilities = params.capabilities;

	const result: InitializeResult = {
		capabilities: {
			// textDocumentSync: TextDocumentSyncKind.Incremental,
			// Tell the client that this server supports code completion.
			// completionProvider: {
			// 	resolveProvider: true
			// },
			// diagnosticProvider: {
			// 	interFileDependencies: false,
			// 	workspaceDiagnostics: false
			// }
		}
	};
	return result;
});

connection.onInitialized(() => {
});

documents.listen(connection);
connection.listen();
