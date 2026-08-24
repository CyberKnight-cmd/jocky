import { useState } from "react";
import Editor from "@monaco-editor/react";
import {
  Play,
  Terminal,
  Trash2,
  CircleCheck,
  CircleX,
  RotateCcw,
} from "lucide-react";
import "./App.css";

const starterCode = `// JOCKY Interactive Output Demo

fn main() -> void {
    log.info("Starting authorization loop...");
    
    int auth_attempts = 0;
    while auth_attempts < 3 {
        log.info("Attempting login...");
        auth_attempts = auth_attempts + 1;
    }
    
    log.info("Locking account!");

    log.info("-------------------------");
    log.info("Forensic Analysis Summary:");
    log.info("Hash mask applied: 0b0010_0000");
    log.info("Shellcode length: 3 bytes");
    log.info("System secured.");
}`;

function App() {
  const [code, setCode] = useState(starterCode);
  const [output, setOutput] = useState("");
  const [running, setRunning] = useState(false);
  const [success, setSuccess] = useState(null);

  const runCode = async () => {
    setRunning(true);
    setOutput("");
    setSuccess(null);

    try {
      const response = await fetch("http://localhost:5000/run", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify({
          code: code,
        }),
      });

      const data = await response.json();

      setOutput(data.output || "No output.");
      setSuccess(data.success);
    } catch (error) {
      setOutput(
        "Could not connect to the Jockey compiler.\n\n" +
        "Make sure the Node server is running."
      );
      setSuccess(false);
    } finally {
      setRunning(false);
    }
  };

  const clearOutput = () => {
    setOutput("");
    setSuccess(null);
  };

  const resetCode = () => {
    setCode(starterCode);
    clearOutput();
  };

  return (
    <div className="app">
      <header className="topbar">
        <div className="brand">
          <div className="brand-icon">
            <Terminal size={25} />
          </div>
          <div>
            <h1>Jockey IDE</h1>
            <p>
              Programming &amp; Forensic Analysis Environment
            </p>
          </div>

        </div>
        <div className="connection">
          <span className="connection-dot"></span>Local Compiler
        </div>
      </header>


      <main className="workspace">
         <section className="panel editor-panel">
          <div className="panel-header">
            <div className="file-name">
              <span className="file-icon">JK</span>
              program.jky
            </div>
            <span className="badge">JOCKEY</span>
          </div>

          <div className="editor-container">
            <Editor
              height="100%"
              defaultLanguage="plaintext"
              value={code}
              onChange={(value) => setCode(value || "")}
              theme="vs-dark"
              options={{
                fontSize: 15,
                minimap: {
                  enabled: false,
                },
                automaticLayout: true,
                scrollBeyondLastLine: false,
                wordWrap: "on",
                padding: {
                  top: 18,
                  bottom: 18,
                },
              }}
            />
          </div>


          <div className="editor-footer">
            <span>Jockey</span>
            <span>UTF-8</span>
            <span>LF</span>
            <span>Local compiler</span>
          </div>
        </section>


        <section className="panel output-panel">
          <div className="panel-header">
            <div className="title"> <Terminal size={18} /> OUTPUT</div>
            <button className="clear-button" onClick={clearOutput}> <Trash2 size={15} />Clear </button>
          </div>

          <div className="output-container">
            {output === "" ? (
              <div className="empty">
                <Terminal size={30} />
                <span> Run your Jockey program to see output here.</span>
              </div>
            ) : (
              <div className="output-content">
                <div className={`result-status ${ success ? "success" : "error"}`}>
                  {success ? (
                    <> <CircleCheck size={16} /> Execution completed </>
                  ) : (
                    <> <CircleX size={16} /> Execution failed</>
                  )}
                </div>
                <pre>{output}</pre>
              </div>
            )}
          </div>


          <div className="actions">
            <button className="run-button" onClick={runCode} disabled={running}>
              <Play size={18} fill="currentColor" />
              {running ? "RUNNING..." : "RUN PROGRAM"}
            </button>
            <button className="reset-button" onClick={resetCode}> <RotateCcw size={17} /> RESET </button>
          </div>
        </section>
      </main>

      <footer>
        <span> Jockey Programming Language </span>

        <span>•</span>

        <span> Local Development Prototype</span>
      </footer>
    </div>
  );
}

export default App;