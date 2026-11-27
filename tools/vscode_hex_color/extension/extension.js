const vscode = require('vscode');

const RE = /\b0x([0-9a-fA-F]{6})\b/g;

function activate(context) {
  const pattern = vscode.workspace.getConfiguration('hex0xColor').get('include');
  const provider = {
    provideDocumentColors(document) {
      const text = document.getText();
      const out = [];
      let m;
      RE.lastIndex = 0;
      while ((m = RE.exec(text))) {
        const v = parseInt(m[1], 16);
        const range = new vscode.Range(document.positionAt(m.index), document.positionAt(m.index + m[0].length));
        out.push(new vscode.ColorInformation(range,
          new vscode.Color(((v >> 16) & 255) / 255, ((v >> 8) & 255) / 255, (v & 255) / 255, 1)));
      }
      return out;
    },
    provideColorPresentations(color, ctx) {
      const hex = (c) => Math.round(c * 255).toString(16).padStart(2, '0');
      let s = hex(color.red) + hex(color.green) + hex(color.blue);
      const orig = ctx.document.getText(ctx.range);
      if (/[A-F]/.test(orig) && !/[a-f]/.test(orig)) s = s.toUpperCase();
      return [new vscode.ColorPresentation('0x' + s)];
    },
  };
  context.subscriptions.push(vscode.languages.registerColorProvider({ scheme: 'file', pattern }, provider));
}

module.exports = { activate, deactivate() {} };
