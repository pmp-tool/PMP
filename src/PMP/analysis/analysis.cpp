#include "CodeObject.h"
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_function.h"
#include "CFG.h"
#include "Instruction.h"
#include <unordered_map>

//#define DEBUG

using namespace std;
using namespace Dyninst;
using namespace ParseAPI;

int main(int argc, char **argv) {

  char buff_jmptab[128];
  sprintf(buff_jmptab, "%s/data/jmptab", getenv("WORKDIR"));

  char buff_calltab[128];
  sprintf(buff_calltab, "%s/data/calltab", getenv("WORKDIR"));

  char buff_insn[128];
  sprintf(buff_insn, "%s/data/insn.all", getenv("WORKDIR"));

  char buff_cg[128];
  sprintf(buff_cg, "%s/data/cg.all", getenv("WORKDIR"));

  char buff_cfg[128];
  sprintf(buff_cfg, "%s/data/cfg.all", getenv("WORKDIR"));

  char buff_log[128];
  sprintf(buff_log, "%s.log", argv[1]);

  char buff_tab[128];
  sprintf(buff_tab, "%s.tab", argv[1]);

  char buff_type[128];
  sprintf(buff_type, "%s/data/type", getenv("WORKDIR"));

  char buffer[1024];
  unordered_map< string, vector<string> > func2tp;
  unordered_map< string, vector<string> > func2dbg;

  if (access(buff_log, F_OK) != -1) {

    FILE *log = fopen(buff_log, "r");

    string prev_tp;
    string prev_dbg;
    bool count_tp = false;
    bool count_dbg = false;

    while (fgets(buffer, sizeof(buffer), log) != NULL) {

      char *start, *end;

      start = buffer;
      end = strchr(start, ' ');
      *end = '\0';
      string func(start);

      start = end + 1;
      end = strchr(start, ' ');
      *end = '\0';
      string tp(start);

      start = end + 1;
      string dbg(start);

      if (func != prev_tp) {

        count_tp = false;

        if (func2tp.find(func) == func2tp.end()) {

          vector<string> tmp;
          func2tp.insert(pair< string, vector<string> >(func, tmp));
          count_tp = true;

        }

      }

      if (count_tp) func2tp.find(func)->second.push_back(tp);     
      prev_tp = func;

      if (func != prev_dbg) {

        count_dbg = false;

        if (func2dbg.find(func) == func2dbg.end()) {

          vector<string> tmp;
          func2dbg.insert(pair< string, vector<string> >(func, tmp));
          count_dbg = true;

        }

      }

      if (count_dbg) func2dbg.find(func)->second.push_back(dbg);
      prev_dbg = func;

    }

    fclose(log);

  }

  FILE *jmptab = fopen(buff_jmptab, "w+");
  FILE *calltab = fopen(buff_calltab, "w+");
  FILE *insn = fopen(buff_insn, "w+");
  FILE *cg = fopen(buff_cg, "w+");
  FILE *cfg = fopen(buff_cfg, "w+");
  FILE *type = fopen(buff_type, "w+");

  if (access(buff_tab, F_OK) != -1 && getenv("DO_CALLTAB") && strcmp(getenv("DO_CALLTAB"), "False") == 0) {

    FILE *tab = fopen(buff_tab, "r");

    while (fgets(buffer, sizeof(buffer), tab) != NULL) {

      char *start, *end;

      start = buffer;
      end = strchr(start, '$');
      *end = '\0';
      string caller_site(start);

      start = end + 1;
      end = strchr(start, '\n');
      *end = '\0';
      string callee_entry(start);

      fprintf(calltab, "%s$%s\n", caller_site.c_str(), callee_entry.c_str());
      fprintf(cg, "%s->%s\n", caller_site.c_str(), callee_entry.c_str());

    }

    fclose(tab);

  }

  Function *main = NULL;

  BPatch bpatch;
  BPatch_binaryEdit *appBin = bpatch.openBinary(argv[1]);
  BPatch_image *appImage = appBin->getImage();

  vector<BPatch_module*> *modules = appImage->getModules();

  for (auto mit = appImage->getModules()->begin(); mit != appImage->getModules()->end(); ++mit) {

    BPatch_module *module = *mit;

    vector<BPatch_function*> *functions = module->getProcedures();

    for (auto fit = functions->begin(); fit != functions->end(); ++fit) {

      string func_name = (*fit)->getMangledName();

      if (func_name.find("_Z") == 0 && func_name.find("C1") != string::npos) 
        func_name.replace(func_name.find("C1"), 2, "C2");

      if (func_name.find("_Z") == 0 && func_name.find("D1") != string::npos) 
        func_name.replace(func_name.find("D1"), 2, "D2");

      Function *func = Dyninst::ParseAPI::convert(*fit);

      size_t memop_count = 0;
      vector<long> memops;

#ifdef DEBUG
      printf("func: %p %s %d\n", func->addr(), func_name.c_str(), func->src());
#endif

      if (func->src() != HINT) continue;

      if (func_name == "main") main = func;

      for (auto eit = func->callEdges().begin(); eit != func->callEdges().end(); ++eit) {

        Edge *edge = *eit;

        if (edge->trg()->start() == -1) continue;

        fprintf(cg, "%lx->%lx\n", edge->src()->last(), edge->trg()->start());

      }

      for (auto bit = func->blocks().begin(); bit != func->blocks().end(); ++bit) {

        Block *block = *bit;

        for (auto eit = block->targets().begin(); eit != block->targets().end(); ++eit) {

          Edge *edge = *eit;
 
          if (edge->trg()->start() == -1 /*|| edge->type() == CALL || edge->type() == RET*/) continue;

          fprintf(cfg, "%lx->%lx\n", edge->src()->start(), edge->trg()->start());

          if (edge->type() == INDIRECT) fprintf(jmptab, "%lx#%lx\n", block->last(), edge->trg()->start());

        }

        Block::Insns insns;
        block->getInsns(insns);

        for (auto iit = insns.begin(); iit != insns.end(); ++iit) {

          if (iit->second.getOperation().getID() == e_fnop) {

            if (type) memops.push_back(iit->first+2);
            //continue;

          }

          fprintf(insn, "%lx", iit->first);
          fprintf(insn, "||%s", iit->second.format().c_str());
          if (iit->first == block->start()) fprintf(insn, "||b");
          if (iit->first == func->addr()) fprintf(insn, "||f||%s", func_name.c_str());
          fprintf(insn, "\n");

        }

      }

      if (memops.size() > 0) {

        char key[256];
        sprintf(key, "%s:%ld", func_name.c_str(), memops.size());
  
        auto tp = func2tp.find(key);
        auto dbg = func2dbg.find(key);

        bool error = false;

        if (tp == func2tp.end() || dbg == func2dbg.end()) {

          error = true;

#ifdef DEBUG
          printf("ERROR2: %p %s %s %ld\n", func->addr(), func_name.c_str(), key, memops.size());
#endif

        } else if (tp->second.size() != memops.size()) {

          error = true;

#ifdef DEBUG
          printf("ERROR1: %p %s %s %ld %ld\n", func->addr(), func_name.c_str(), key, tp->second.size(), memops.size());
#endif

        } else {

#ifdef DEBUG
          printf("SUCCESS: %p %s %s %ld\n", func->addr(), func_name.c_str(), key, memops.size());
#endif

        }

        if (!error) {

          for (int index = 0; index < memops.size(); index++) {

            fprintf(type, "%x %s %s", memops[index], tp->second[index].c_str(), dbg->second[index].c_str());

          }

        }

      }

    }

  } 

  if (main) fprintf(cg, "main:%lx\n", main->addr());

  fclose(jmptab);  
  fclose(calltab);
  fclose(insn);
  fclose(cg);
  fclose(cfg);
  fclose(type);

#ifdef DEBUG
  printf("Analysis Done!\n");
#endif

}
