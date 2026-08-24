const express = require("express");
const cors = require("cors");
const fs = require("fs");
const os = require("os");
const path = require("path");
const crypto = require("crypto");
const { spawn } = require("child_process");

const app = express();

const PORT = 5000;
const JKY_COMMAND = path.join(__dirname, "../../bin/jky.exe");

app.use(cors());
app.use(express.json());

app.post("/run", (req, res) => {
  const { code } = req.body;

  if (!code || !code.trim()) {
    return res.json({
      success: false,
      output: "No Jockey code provided.",
    });
  }

  const fileName = `jockey_${crypto.randomUUID()}.jky`;
  const filePath = path.join(os.tmpdir(), fileName);

  try {
    fs.writeFileSync(filePath, code, "utf8");

    console.log(`Running Jockey file: ${filePath}`);

    const compiler = spawn(JKY_COMMAND, ["run", filePath], {
      shell: false,
      cwd: os.tmpdir(),
    });

    let output = "";
    let errorOutput = "";

    compiler.stdout.on("data", (data) => {
      output += data.toString();
    });

    compiler.stderr.on("data", (data) => {
      errorOutput += data.toString();
    });

    compiler.on("error", (error) => {
      fs.unlink(filePath, () => {});

      res.status(500).json({
        success: false,
        output:
          "Could not start the Jockey compiler.\n\n" +
          error.message +
          "\n\n" +
          "Make sure the 'jky' command works in the terminal.",
      });
    });

    compiler.on("close", (exitCode) => {
      fs.unlink(filePath, () => {});

      if (exitCode === 0) {
        res.json({
          success: true,
          output: output || "Program finished with no output.",
        });
      } else {
        res.json({
          success: false,
          output:
            errorOutput ||
            output ||
            `Jockey program exited with code ${exitCode}.`,
        });
      }
    });
  } catch (error) {
    fs.unlink(filePath, () => {});

    res.status(500).json({
      success: false,
      output: error.message,
    });
  }
});

app.listen(PORT, () => {
  console.log(`Jockey backend running on http://localhost:${PORT}`);
});