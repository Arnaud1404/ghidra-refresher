// Prints every non-external function with its decompiled C (headless check).
import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.program.model.listing.Function;

public class DumpDecomp extends GhidraScript {
  @Override
  public void run() throws Exception {
    DecompInterface ifc = new DecompInterface();
    ifc.openProgram(currentProgram);
    println("LANG " + currentProgram.getLanguageID());
    for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
      if (f.isExternal() || f.isThunk()) continue;
      DecompileResults r = ifc.decompileFunction(f, 60, monitor);
      println("=== " + f.getName() + " @ " + f.getEntryPoint());
      if (r.decompileCompleted()) println(r.getDecompiledFunction().getC());
    }
  }
}
