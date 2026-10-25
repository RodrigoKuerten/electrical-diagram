// Decompile selected functions or all functions into a directory.
//@category REA
import ghidra.app.decompiler.*;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.*;
import java.io.File;
import java.io.PrintWriter;

public class DecompileTargets extends GhidraScript {
    @Override
    public void run() throws Exception {
        String[] args = getScriptArgs();
        if (args.length < 1) {
            println("usage: DecompileTargets <output-dir> [address ...]");
            return;
        }
        File outDir = new File(args[0]);
        outDir.mkdirs();
        DecompInterface decomp = new DecompInterface();
        decomp.toggleCCode(true);
        decomp.toggleSyntaxTree(true);
        decomp.setSimplificationStyle("decompile");
        if (!decomp.openProgram(currentProgram)) {
            println("cannot open program: " + decomp.getLastMessage());
            return;
        }
        FunctionManager fm = currentProgram.getFunctionManager();
        java.util.List<Function> targets = new java.util.ArrayList<>();
        if (args.length > 1) {
            for (int i = 1; i < args.length; i++) {
                Address a = toAddr(args[i]);
                Function f = fm.getFunctionContaining(a);
                if (f == null) {
                    println("no function containing " + args[i]);
                } else {
                    targets.add(f);
                }
            }
        } else {
            for (Function f : fm.getFunctions(true)) {
                targets.add(f);
            }
        }
        int ok = 0;
        for (Function f : targets) {
            monitor.checkCancelled();
            DecompileResults res = decomp.decompileFunction(f, 180, monitor);
            String name = f.getName() + "_" + f.getEntryPoint().toString().replace("0x", "");
            File out = new File(outDir, name + ".c");
            try (PrintWriter pw = new PrintWriter(out, "UTF-8")) {
                pw.println("// " + f.getName() + " @ " + f.getEntryPoint());
                if (res.decompileCompleted()) {
                    pw.println(res.getDecompiledFunction().getC());
                    ok++;
                } else {
                    pw.println("// DECOMPILE FAILED: " + res.getErrorMessage());
                }
            }
        }
        println("decompiled " + ok + "/" + targets.size() + " functions to " + outDir);
        decomp.dispose();
    }
}
