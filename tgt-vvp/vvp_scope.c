/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifdef HAVE_CVS_IDENT
#ident "$Id: vvp_scope.c,v 1.78 2002/09/17 05:37:45 steve Exp $"
#endif

# include  "vvp_priv.h"
# include  <assert.h>
#ifdef HAVE_MALLOC_H
# include  <malloc.h>
#endif
# include  <stdlib.h>
# include  <string.h>

/*
 *  Escape non-symbol chararacters in ids, and quotes in strings.
 */

inline static char hex_digit(unsigned i)
{
      i &= 0xf;
      return i>=10 ? i-10+'A' : i+'0';
}

const char *vvp_mangle_id(const char *id)
{
      static char *out = 0x0;
      static size_t out_len;
      
      int nesc = 0;
      int iout = 0;
      const char *inp = id;

      const char nosym[] = "!\"#%&'()*+,-/:;<=>?@[\\]^`{|}~";
      
      char *se = strpbrk(inp, nosym);
      if (!se)
	    return id;
      
      do {
	    int n = se - inp;
	    int nlen = strlen(id) + 4*(++nesc) + 1;
	    if (out_len < nlen) {
		  out = (char *) realloc(out, nlen);
		  assert(out);
		  out_len = nlen;
	    }
	    if (n) {
		  strncpy(out+iout, inp, n);
		  iout += n;
	    }
	    inp += n+1;
	    out[iout++] = '\\';
	    switch (*se) {
		case '\\':
		case '/':
		case '<':
		case '>':
		  out[iout++] = *se;
		  break;
		default:
		  out[iout++] = 'x';
		  out[iout++] = hex_digit(*se >> 4);
		  out[iout++] = hex_digit(*se);
		  break;
	    }

	    se = strpbrk(inp, nosym);
      } while (se);
      
      strcpy(out+iout, inp);
      return out;
}

const char *vvp_mangle_name(const char *id)
{
      static char *out = 0x0;
      static size_t out_len;
      
      int nesc = 0;
      int iout = 0;
      const char *inp = id;

      const char nosym[] = "\"\\";
      
      char *se = strpbrk(inp, nosym);
      if (!se)
	    return id;
      
      do {
	    int n = se - inp;
	    int nlen = strlen(id) + 2*(++nesc) + 1;
	    if (out_len < nlen) {
		  out = (char *) realloc(out, nlen);
		  assert(out);
		  out_len = nlen;
	    }
	    if (n) {
		  strncpy(out+iout, inp, n);
		  iout += n;
	    }
	    inp += n+1;
	    out[iout++] = '\\';
	    out[iout++] = *se;

	    se = strpbrk(inp, nosym);
      } while (se);
      
      strcpy(out+iout, inp);
      return out;
}

/*
 * Given a signal, generate a string name that is suitable for use as
 * a label. The only rule is that the same signal will always have the
 * same label. The result is stored in static memory, so remember to
 * copy it out.
 */
const char* vvp_signal_label(ivl_signal_t sig)
{
      static char buf[32];
      sprintf(buf, "$%p", sig);
      return buf;
}

/*
 * This makes a string suitable for use as a label for memories.
 */
const char* vvp_memory_label(ivl_memory_t mem)
{
      static char buf[32];
      sprintf(buf, "$%p", mem);
      return buf;
}

ivl_signal_type_t signal_type_of_nexus(ivl_nexus_t nex)
{
      unsigned idx;
      ivl_signal_type_t out = IVL_SIT_TRI;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_signal_type_t stype;
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_signal_t sig = ivl_nexus_ptr_sig(ptr);
	    if (sig == 0)
		  continue;

	    stype = ivl_signal_type(sig);
	    if (stype == IVL_SIT_REG)
		  continue;
	    if (stype == IVL_SIT_TRI)
		  continue;
	    if (stype == IVL_SIT_NONE)
		  continue;
	    out = stype;
      }

      return out;
}

ivl_nexus_ptr_t ivl_logic_pin_ptr(ivl_net_logic_t net, unsigned pin)
{
      ivl_nexus_t nex = ivl_logic_pin(net, pin);
      unsigned idx;

      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex, idx);
	    ivl_net_logic_t tmp = ivl_nexus_ptr_log(ptr);
	    if (tmp == 0)
		  continue;
	    if (tmp != net)
		  continue;
	    if (ivl_nexus_ptr_pin(ptr) != pin)
		  continue;

	    return ptr;
      }
      assert(0);
      return 0;
}

const char*drive_string(ivl_drive_t drive)
{
      switch (drive) {
	  case IVL_DR_HiZ:
	    return "";
	  case IVL_DR_SMALL:
	    return "sm";
	  case IVL_DR_MEDIUM:
	    return "me";
	  case IVL_DR_WEAK:
	    return "we";
	  case IVL_DR_LARGE:
	    return "la";
	  case IVL_DR_PULL:
	    return "pu";
	  case IVL_DR_STRONG:
	    return "";
	  case IVL_DR_SUPPLY:
	    return "su";
      }

      return "";
}

	    
/*
 * The draw_scope function draws the major functional items within a
 * scope. This includes the scopes themselves, of course. All the
 * other functions in this file are in support of that task.
 */


/*
 * NEXUS
 * ivl builds up the netlist into objects connected together by
 * ivl_nexus_t objects. The nexus receives all the drivers of the
 * point in the net and resolves the value. The result is then sent to
 * all the nets that are connected to the nexus. The nets, then, are
 * read to get the value of the nexus.
 *
 * NETS
 * Nets are interesting and special, because a nexus may be connected
 * to several of them at once. This can happen, for example, as an
 * artifact of module port connects, where the inside and the outside
 * of the module are connected through an in-out port. (In fact, ivl
 * will simply connect signals that are bound through a port, because
 * the input/output/inout properties are enforced as compile time.)
 *
 * This case is handled by choosing one to receive the value of the
 * nexus. This one then feeds to another net at the nexus, and so
 * on. The last net is selected as the output of the nexus.
 */
/*
 * This function takes a nexus and looks for an input functor. It then
 * draws to the output a string that represents that functor. What we
 * are trying to do here is find the input to the net that is attached
 * to this nexus.
 */

static const char* draw_net_input_drive(ivl_nexus_t nex, ivl_nexus_ptr_t nptr)
{
      static char result[2048];
      unsigned idx;
      unsigned nptr_pin = ivl_nexus_ptr_pin(nptr);
      ivl_net_const_t cptr;
      ivl_net_logic_t lptr;
      ivl_signal_t sptr;
      ivl_lpm_t lpm;

      lptr = ivl_nexus_ptr_log(nptr);
      if (lptr && (ivl_logic_type(lptr) == IVL_LO_BUFZ) && (nptr_pin == 0))
	    do {
		  if (ivl_nexus_ptr_drive0(nptr) != IVL_DR_STRONG)
			break;

		  if (ivl_nexus_ptr_drive1(nptr) != IVL_DR_STRONG)
			break;

		  if (ivl_logic_delay(lptr, 0) != 0)
			break;

		  if (nex == ivl_logic_pin(lptr, 1))
			break;

		  return draw_net_input(ivl_logic_pin(lptr, 1));
	    } while(0);

      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLDOWN)) {
	    return "C<pu0>";
      }

      if (lptr && (ivl_logic_type(lptr) == IVL_LO_PULLUP)) {
	    return "C<pu1>";
      }

      if (lptr && (nptr_pin == 0)) {
	    sprintf(result, "L_%s", vvp_mangle_id(ivl_logic_name(lptr)));
	    return result;
      }

      sptr = ivl_nexus_ptr_sig(nptr);
      if (sptr && (ivl_signal_type(sptr) == IVL_SIT_REG)) {
	    sprintf(result, "V_%s[%u]", vvp_signal_label(sptr), nptr_pin);
	    return result;
      }

      cptr = ivl_nexus_ptr_con(nptr);
      if (cptr) {
	    const char*bits = ivl_const_bits(cptr);
	    ivl_drive_t drive;
	    switch (bits[nptr_pin]) {
		case '0':
		  drive = ivl_nexus_ptr_drive0(nptr);
		  if (drive == IVL_DR_HiZ)
			sprintf(result, "C<z>");
		  else
			sprintf(result, "C<%s0>", drive_string(drive));
		  break;
		case '1':
		  drive = ivl_nexus_ptr_drive1(nptr);
		  if (drive == IVL_DR_HiZ)
			sprintf(result, "C<z>");
		  else
			sprintf(result, "C<%s1>", drive_string(drive));
		  break;
		default:
		  sprintf(result, "C<%c>", bits[nptr_pin]);
	    }

	    return result;
      }

      lpm = ivl_nexus_ptr_lpm(nptr);
      if (lpm) switch (ivl_lpm_type(lpm)) {

	  case IVL_LPM_FF:
	  case IVL_LPM_MUX:
	    for (idx = 0 ;  idx < ivl_lpm_width(lpm) ;  idx += 1)
		  if (ivl_lpm_q(lpm, idx) == nex) {
			sprintf(result, "L_%s/%u",
				vvp_mangle_id(ivl_lpm_name(lpm)), idx);
			return result;
		  }
	    break;

	  case IVL_LPM_RAM:
	  case IVL_LPM_ADD:
	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	  case IVL_LPM_SUB:
	  case IVL_LPM_MULT:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	  case IVL_LPM_UFUNC:
	    for (idx = 0 ;  idx < ivl_lpm_width(lpm) ;  idx += 1)
		  if (ivl_lpm_q(lpm, idx) == nex) {
			sprintf(result, "L_%s[%u]",
				vvp_mangle_id(ivl_lpm_name(lpm)), idx);
			return result;
		  }

	    break;


	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_NE:
	    if (ivl_lpm_q(lpm, 0) == nex) {
		  sprintf(result, "L_%s", vvp_mangle_id(ivl_lpm_name(lpm)));
		  return result;
	    }
	    break;

      }

      fprintf(stderr, "internal error: no input to nexus %s\n",
	      ivl_nexus_name(nex));
      assert(0);
      return "C<z>";
}

/*
 * This function draws the input to a net. What that means is that it
 * returns a static string that can be used to represent a resolved
 * driver to a nexus. If there are multiple drivers to the nexus, then
 * it writes out the resolver declarations needed to perform strength
 * resolution.
 *
 * The string that this returns is bound to the nexus, so the pointer
 * remains valid.
 */
const char* draw_net_input(ivl_nexus_t nex)
{
      ivl_signal_type_t res;
      char result[512];
      unsigned idx;
      int level;
      unsigned ndrivers = 0;
      static ivl_nexus_ptr_t *drivers = 0x0;
      static unsigned adrivers = 0;

      const char*resolv_type;

	/* If this nexus already has a label, then its input is
	   already figured out. Just return the existing label. */
      char*nex_private = (char*)ivl_nexus_get_private(nex);
      if (nex_private)
	    return nex_private;

      res = signal_type_of_nexus(nex);
      switch (res) {
	  case IVL_SIT_TRI:
	    resolv_type = "tri";
	    break;
	  case IVL_SIT_TRI0:
	    resolv_type = "tri0";
	    break;
	  case IVL_SIT_TRI1:
	    resolv_type = "tri1";
	    break;

	      /* Catch the special cases that the nets are supply
		 nets. Drive constant values uncomditionally. */
	  case IVL_SIT_SUPPLY0:
	    nex_private = "C<su0>";
	    ivl_nexus_set_private(nex, nex_private);
	    return nex_private;
	  case IVL_SIT_SUPPLY1:
	    nex_private = "C<su1>";
	    ivl_nexus_set_private(nex, nex_private);
	    return nex_private;

	  default:
	    fprintf(stderr, "vvp.tgt: Unsupported signal type: %u\n", res);
	    assert(0);
	    resolv_type = "tri";
	    break;
      }


      for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	    ivl_nexus_ptr_t nptr = ivl_nexus_ptr(nex, idx);

	      /* Skip input only pins. */
	    if ((ivl_nexus_ptr_drive0(nptr) == IVL_DR_HiZ)
		&& (ivl_nexus_ptr_drive1(nptr) == IVL_DR_HiZ))
		  continue;

	      /* Save this driver. */
	    if (ndrivers >= adrivers) {
		  adrivers += 4;
		  drivers = (ivl_nexus_ptr_t*)
			realloc(drivers, adrivers*sizeof(ivl_nexus_ptr_t));
		  assert(drivers);
	    }
	    drivers[ndrivers] = nptr;
	    ndrivers += 1;
      }

	/* If the nexus has no drivers, then send a constant HiZ into
	   the net. */
      if (ndrivers == 0) {
	    switch (res) {
		case IVL_SIT_TRI:
		  nex_private = "C<z>";
		  break;
		case IVL_SIT_TRI0:
		  nex_private = "C<0>";
		  break;
		case IVL_SIT_TRI1:
		  nex_private = "C<1>";
		  break;
		default:
		  assert(0);
	    }
	    ivl_nexus_set_private(nex, nex_private);
	    return nex_private;
      }


	/* If the nexus has exactly one driver, then simply draw
	   it. Note that this will *not* work if the nexus is not a
	   TRI type nexus. */
      if (ndrivers == 1 && res == IVL_SIT_TRI) {
	    nex_private = strdup(draw_net_input_drive(nex, drivers[0]));
	    ivl_nexus_set_private(nex, nex_private);
	    return nex_private;
      }

      level = 0;
      while (ndrivers) {
	    int inst;
	    for (inst = 0; inst < ndrivers; inst += 4) {
		  if (ndrivers > 4)
			fprintf(vvp_out, "RS_%s/%d/%d .resolv tri", 
				vvp_mangle_id(ivl_nexus_name(nex)),
				level, inst);
		  else 
			fprintf(vvp_out, "RS_%s .resolv %s", 
				vvp_mangle_id(ivl_nexus_name(nex)),
				resolv_type);
		  
		  for (idx = inst; idx < ndrivers && idx < inst+4; idx += 1) {
			if (level) {
			      fprintf(vvp_out, ", RS_%s/%d/%d",
				      vvp_mangle_id(ivl_nexus_name(nex)),
				      level - 1,
				      idx*4);
			} else {
			      fprintf(vvp_out, ", %s",
				      draw_net_input_drive(nex, drivers[idx]));
			}
		  }
		  for ( ;  idx < inst+4 ;  idx += 1)
			fprintf(vvp_out, ", C<z>");
		  
		  fprintf(vvp_out, ";\n");
	    }
	    if (ndrivers > 4)
		  ndrivers = (ndrivers+3) / 4;
	    else
		  ndrivers = 0;
	    level += 1;
      }
      
      sprintf(result, "RS_%s", vvp_mangle_id(ivl_nexus_name(nex)));
      nex_private = strdup(result);
      ivl_nexus_set_private(nex, nex_private);
      return nex_private;
}



/*
 * This function looks at the nexus in search of the net to attach
 * functor inputs to. Sort the signals in the nexus by name, and
 * choose the lexically earliest one.
 */
void draw_input_from_net(ivl_nexus_t nex)
{
      const char*nex_private = (const char*)ivl_nexus_get_private(nex);
      if (nex_private == 0)
	    nex_private = draw_net_input(nex);
      assert(nex_private);
      fprintf(vvp_out, "%s", nex_private);
}

/*
 * This function draws a reg/int/variable in the scope. This is a very
 * simple device to draw as there are no inputs to connect so no need
 * to scan the nexus.
 */
static void draw_reg_in_scope(ivl_signal_t sig)
{
      int msb = ivl_signal_pins(sig) - 1;
      int lsb = 0;

      const char*signed_flag = ivl_signal_integer(sig) ? "/i" :
			       ivl_signal_signed(sig)? "/s" : "";

      fprintf(vvp_out, "V_%s .var%s \"%s\", %d, %d;\n",
	      vvp_signal_label(sig), signed_flag,
	      vvp_mangle_name(ivl_signal_basename(sig)), msb, lsb);
}

/*
 * This function draws a net. This is a bit more complicated as we
 * have to find an appropriate functor to connect to the input.
 */
static void draw_net_in_scope(ivl_signal_t sig)
{
      unsigned idx;
      int msb = ivl_signal_pins(sig) - 1;
      int lsb = 0;
      typedef const char*const_charp;
      const_charp* args;

      const char*signed_flag = ivl_signal_signed(sig)? "/s" : "";

	/* Skip the local signal. */
      if (ivl_signal_local(sig))
	    return;

      args = (const_charp*)calloc(ivl_signal_pins(sig), sizeof(char*));

	/* Connect all the pins of the signal to something. */
      for (idx = 0 ;  idx < ivl_signal_pins(sig) ;  idx += 1) {
	    ivl_nexus_t nex = ivl_signal_pin(sig, idx);

	    args[idx] = draw_net_input(nex);
      }

      fprintf(vvp_out, "V_%s .net%s \"%s\", %d, %d",
	      vvp_signal_label(sig), signed_flag,
	      vvp_mangle_name(ivl_signal_basename(sig)), msb, lsb);
      for (idx = 0 ;  idx < ivl_signal_pins(sig) ;  idx += 1) {
	    fprintf(vvp_out, ", %s", args[idx]);
      }
      fprintf(vvp_out, ";\n");

      free(args);
}

static void draw_delay(ivl_net_logic_t lptr)
{
      unsigned d0 = ivl_logic_delay(lptr, 0);
      unsigned d1 = ivl_logic_delay(lptr, 1);
      unsigned d2 = ivl_logic_delay(lptr, 2);
      
      if (d0 == 0 && d1 == 0 && d2 == 0)
	    return;

      if (d0 == d1 && d1 == d2)
	    fprintf(vvp_out, " (%d)", d0);
      else
	    fprintf(vvp_out, " (%d,%d,%d)", d0, d1, d2);
}

static void draw_udp_def(ivl_udp_t udp)
{
  unsigned init;
  int i;

  switch (ivl_udp_init(udp))
    {
    case '0':
      init = 0;
      break;
    case '1':
      init = 1;
      break;
    default:
      init = 2;
      break;
    }

  if (ivl_udp_sequ(udp))
	fprintf(vvp_out, 
		"UDP_%s .udp/sequ \"%s\", %d, %d",
		vvp_mangle_id(ivl_udp_name(udp)),
		vvp_mangle_name(ivl_udp_name(udp)),
		ivl_udp_nin(udp),
		init );
  else
	fprintf(vvp_out, 
		"UDP_%s .udp/comb \"%s\", %d",
		vvp_mangle_id(ivl_udp_name(udp)),
		vvp_mangle_name(ivl_udp_name(udp)),
		ivl_udp_nin(udp));

  for (i=0; i<ivl_udp_rows(udp); i++)
    fprintf(vvp_out, "\n ,\"%s\"", ivl_udp_row(udp, i) );

  fprintf(vvp_out, ";\n");
}

static void draw_udp_in_scope(ivl_net_logic_t lptr)
{
  unsigned pdx;

  ivl_udp_t udp = ivl_logic_udp(lptr);
  
  static ivl_udp_t *udps = 0x0;
  static int nudps = 0;
  int i;

  for (i=0; i<nudps; i++)
    if (udps[i] == udp)
      break;
  
  if (i >= nudps)
    {
      udps = (ivl_udp_t*)realloc(udps, (nudps+1)*sizeof(ivl_udp_t));
      assert(udps);
      udps[nudps++] = udp;
      draw_udp_def(udp);
    }

  fprintf(vvp_out, "L_%s .udp",
	  vvp_mangle_id(ivl_logic_name(lptr)));
  fprintf(vvp_out, " UDP_%s", 
	  vvp_mangle_id(ivl_udp_name(udp)));
  draw_delay(lptr);
  
  for (pdx = 1 ;  pdx < ivl_logic_pins(lptr) ;  pdx += 1) 
    {
      ivl_nexus_t nex = ivl_logic_pin(lptr, pdx);
      fprintf(vvp_out, ", ");
      draw_input_from_net(nex);
    }
  
  fprintf(vvp_out, ";\n");
}

static void draw_logic_in_scope(ivl_net_logic_t lptr)
{
      unsigned pdx;
      const char*ltype = "?";
      const char*lcasc = 0x0;
      char identity_val = '0';

      ivl_drive_t str0, str1;

      int level;
      int ninp = ivl_logic_pins(lptr) - 1;
      typedef const char*const_charp;
      const_charp*input_strings = calloc(ninp, sizeof(const_charp));

      for (pdx = 0 ;  pdx < ninp ;  pdx += 1)
	    input_strings[pdx] = draw_net_input(ivl_logic_pin(lptr, pdx+1));
      
      switch (ivl_logic_type(lptr)) {

          case IVL_LO_UDP:
	    draw_udp_in_scope(lptr);
	    return;

          case IVL_LO_BUFZ: {
		  /* Draw bufz objects, but only if the output drive
		     is different from the input. */
		ivl_nexus_ptr_t nptr = ivl_logic_pin_ptr(lptr,0);
		ivl_drive_t dr0 = ivl_nexus_ptr_drive0(nptr);
		ivl_drive_t dr1 = ivl_nexus_ptr_drive1(nptr);

		ltype = "BUFZ";

		if (dr0 != IVL_DR_STRONG)
			break;

		if (dr1 != IVL_DR_STRONG)
			break;

		if (ivl_logic_delay(lptr, 0) != 0)
		      break;

		if (ivl_logic_pin(lptr, 0) == ivl_logic_pin(lptr, 1))
		      break;

		return;
	  }

	  case IVL_LO_PULLDOWN:
	  case IVL_LO_PULLUP:
	      /* Skip pullup and pulldown objects. Things that have
		 pull objects as inputs will instead generate the
		 appropriate C<?> symbol. */
	    return;

	  case IVL_LO_AND:
	    ltype = "AND";
	    identity_val = '1';
	    break;

	  case IVL_LO_BUF:
	    ltype = "BUF";
	    break;

	  case IVL_LO_BUFIF0:
	    ltype = "BUFIF0";
	    break;

	  case IVL_LO_BUFIF1:
	    ltype = "BUFIF1";
	    break;

	  case IVL_LO_NAND:
	    ltype = "NAND";
	    lcasc = "AND";
	    identity_val = '1';
	    break;

	  case IVL_LO_NOR:
	    ltype = "NOR";
	    lcasc = "OR";
	    break;

	  case IVL_LO_NOT:
	    ltype = "NOT";
	    break;

	  case IVL_LO_OR:
	    ltype = "OR";
	    break;

	  case IVL_LO_XNOR:
	    ltype = "XNOR";
	    lcasc = "XOR";
	    break;

	  case IVL_LO_XOR:
	    ltype = "XOR";
	    break;

	  case IVL_LO_EEQ:
	    ltype = "EEQ";
	    break;

	  case IVL_LO_PMOS:
	    ltype = "PMOS";
	    break;

	  case IVL_LO_NMOS:
	    ltype = "NMOS";
	    break;

	  case IVL_LO_RPMOS:
	    ltype = "RPMOS";
	    break;

	  case IVL_LO_RNMOS:
	    ltype = "RNMOS";
	    break;

	  case IVL_LO_NOTIF0:
	    ltype = "NOTIF0";
	    break;

	  case IVL_LO_NOTIF1:
	    ltype = "NOTIF1";
	    break;

	  default:
	    fprintf(stderr, "vvp.tgt: error: Unhandled logic type: %u\n",
		    ivl_logic_type(lptr));
	    ltype = "?";
	    break;
      }

      { ivl_nexus_t nex = ivl_logic_pin(lptr, 0);
        ivl_nexus_ptr_t nptr = 0;
        unsigned idx;
	for (idx = 0 ;  idx < ivl_nexus_ptrs(nex) ;  idx += 1) {
	      nptr = ivl_nexus_ptr(nex,idx);
	      if (ivl_nexus_ptr_log(nptr) != lptr)
		    continue;
	      if (ivl_nexus_ptr_pin(nptr) != 0)
		    continue;
	      break;
	}
        str0 = ivl_nexus_ptr_drive0(nptr);
	str1 = ivl_nexus_ptr_drive1(nptr);
      }

      if (!lcasc)
	lcasc = ltype;

	/* Get all the input label that I will use for parameters to
	   the functor that I create later. */
      ninp = ivl_logic_pins(lptr) - 1;
      input_strings = calloc(ninp, sizeof(char*));
      for (pdx = 0 ;  pdx < ninp ;  pdx += 1)
	    input_strings[pdx] = draw_net_input(ivl_logic_pin(lptr, pdx+1));
      
      level = 0;
      ninp = ivl_logic_pins(lptr) - 1;
      while (ninp) {
	    int inst;
	    for (inst = 0; inst < ninp; inst += 4) {
		  if (ninp > 4)
			fprintf(vvp_out, "L_%s/%d/%d .functor %s", 
				vvp_mangle_id(ivl_logic_name(lptr)),
				level, inst,
				lcasc);
		  else {
			fprintf(vvp_out, "L_%s .functor %s", 
				vvp_mangle_id(ivl_logic_name(lptr)),
				ltype);

			draw_delay(lptr);

			if (str0 != IVL_DR_STRONG || str1 != IVL_DR_STRONG)
			      fprintf(vvp_out, " [%u %u]", str0, str1);

		  }
		  for (pdx = inst; pdx < ninp && pdx < inst+4 ; pdx += 1) {
			if (level) {
			      fprintf(vvp_out, ", L_%s/%d/%d",
				      vvp_mangle_id(ivl_logic_name(lptr)),
				      level - 1,
				      pdx*4 );
			} else {
			      fprintf(vvp_out, ", %s", input_strings[pdx]);
			}
		  }
		  for ( ;  pdx < inst+4 ;  pdx += 1) {
			fprintf(vvp_out, ", C<%c>", identity_val);
		  }
		  
		  fprintf(vvp_out, ";\n");
	    }
	    if (ninp > 4)
		  ninp = (ninp+3) / 4;
	    else
		  ninp = 0;
	    level += 1;
      }

	/* Free the array of char*. The strings themselves are
	   persistent, held by the ivl_nexus_t objects. */
      free(input_strings);
}

static void draw_event_in_scope(ivl_event_t obj)
{
      unsigned nany = ivl_event_nany(obj);
      unsigned nneg = ivl_event_nneg(obj);
      unsigned npos = ivl_event_npos(obj);

      unsigned cnt = 0;

	/* Figure out how many probe functors are needed. */
      if (nany > 0)
	    cnt += (nany+3) / 4;

      if (nneg > 0)
	    cnt += (nneg+3) / 4;

      if (npos > 0)
	    cnt += (npos+3) / 4;

      if (cnt == 0) {
	      /* If none are needed, then this is a named event. The
		 code needed is easy. */
	    fprintf(vvp_out, "E_%s .event \"%s\";\n",
		    vvp_mangle_id(ivl_event_name(obj)), 
		    vvp_mangle_name(ivl_event_basename(obj)));

      } else if (cnt > 1) {
	    unsigned idx;
	    unsigned ecnt = 0;

	    for (idx = 0 ;  idx < nany ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%s/%u .event edge",
			  vvp_mangle_id(ivl_event_name(obj)), ecnt);
		  
		  top = idx + 4;
		  if (nany < top)
			top = nany;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < nneg ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%s/%u .event negedge",
			  vvp_mangle_id(ivl_event_name(obj)), ecnt);

		  top = idx + 4;
		  if (nneg < top)
			top = nneg;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    for (idx = 0 ;  idx < npos ;  idx += 4, ecnt += 1) {
		  unsigned sub, top;

		  fprintf(vvp_out, "E_%s/%u .event posedge",
			  vvp_mangle_id(ivl_event_name(obj)), ecnt);

		  top = idx + 4;
		  if (npos < top)
			top = npos;
		  for (sub = idx ;  sub < top ;  sub += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, sub);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  fprintf(vvp_out, ";\n");
	    }

	    assert(ecnt == cnt);

	    fprintf(vvp_out, "E_%s .event/or",
		    vvp_mangle_id(ivl_event_name(obj)));
	    fprintf(vvp_out, " E_%s/0", 
		    vvp_mangle_id(ivl_event_name(obj)));

	    for (idx = 1 ;  idx < cnt ;  idx += 1)
		  fprintf(vvp_out, ", E_%s/%u", 
			  vvp_mangle_id(ivl_event_name(obj)), idx);
	    
	    fprintf(vvp_out, ";\n");
	    
      } else {
	    unsigned idx;
	    
	    fprintf(vvp_out, "E_%s .event ", 
		    vvp_mangle_id(ivl_event_name(obj)));

	    if (nany > 0) {
		  assert((nneg + npos) == 0);
		  assert(nany <= 4);
		  
		  fprintf(vvp_out, "edge");
		  
		  for (idx = 0 ;  idx < nany ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_any(obj, idx);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  
	    } else if (nneg > 0) {
		  assert((nany + npos) == 0);
		  fprintf(vvp_out, "negedge");

		  for (idx = 0 ;  idx < nneg ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_neg(obj, idx);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
		  
	    } else {
		  assert((nany + nneg) == 0);
		  fprintf(vvp_out, "posedge");
		  
		  for (idx = 0 ;  idx < npos ;  idx += 1) {
			ivl_nexus_t nex = ivl_event_pos(obj, idx);
			fprintf(vvp_out, ", ");
			draw_input_from_net(nex);
		  }
	    }
	    
	    fprintf(vvp_out, ";\n");
      }
}

inline static void draw_lpm_ram(ivl_lpm_t net)
{
      unsigned idx;
      unsigned width = ivl_lpm_width(net);
      unsigned awidth = ivl_lpm_selects(net);
      ivl_memory_t mem = ivl_lpm_memory(net);
      ivl_nexus_t clk = ivl_lpm_clk(net);
      ivl_nexus_t pin;

      if (clk) {
	    fprintf(vvp_out,
		    "CLK_%s .event posedge, ",
		    vvp_mangle_id(ivl_lpm_name(net)));
	    draw_input_from_net(clk);
	    fprintf(vvp_out, ";\n");
      }

      fprintf(vvp_out, 
	      "L_%s .mem/port", 
	      vvp_mangle_id(ivl_lpm_name(net)));
      fprintf(vvp_out, 
	      " M_%s, %d,0, %d,\n  ", 
	      vvp_memory_label(mem),
	      width-1,
	      awidth);

      for (idx = 0 ;  idx < awidth ;  idx += 1) {
	    pin = ivl_lpm_select(net, idx);
	    if (idx) fprintf(vvp_out, ", ");
	    draw_input_from_net(pin);
      }
      
      if (clk) {
	    fprintf(vvp_out, ",\n  CLK_%s, ", 
		    vvp_mangle_id(ivl_lpm_name(net)));
	    pin = ivl_lpm_enable(net);
	    if (pin)
		  draw_input_from_net(pin);
	    else
		  fprintf(vvp_out, "C<1>");
	    for (idx=0; idx<width; idx++) {
		  pin = ivl_lpm_data(net, idx);
		  fprintf(vvp_out, ", ");
		  draw_input_from_net(pin);
	    }
      }
      
      fprintf(vvp_out, ";\n");
}

static void draw_lpm_arith_a_b_inputs(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned idx;
      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t a = ivl_lpm_data(net, idx);
	    if (a) {
		  fprintf(vvp_out, ", ");
		  draw_input_from_net(a);
	    } else {
		  fprintf(vvp_out, ", C<0>");
	    }
      }

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t b = ivl_lpm_datab(net, idx);
	    if (b) {
		  fprintf(vvp_out, ", ");
		  draw_input_from_net(b);
	    } else {
		  fprintf(vvp_out, ", C<0>");
	    }
      }
}

static void draw_lpm_add(ivl_lpm_t net)
{
      unsigned width;
      const char*type = "";

      width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_ADD:
	    type = "sum";
	    break;
	  case IVL_LPM_SUB:
	    type = "sub";
	    break;
	  case IVL_LPM_MULT:
	    type = "mult";
	    break;
	  case IVL_LPM_DIVIDE:
	    type = "div";
	    break;
	  case IVL_LPM_MOD:
	    type = "mod";
	    break;
	  default:
	    assert(0);
      }

      fprintf(vvp_out, "L_%s .arith/%s %u", 
	      vvp_mangle_id(ivl_lpm_name(net)), type, width);

      draw_lpm_arith_a_b_inputs(net);

      fprintf(vvp_out, ";\n");
}

static void draw_lpm_cmp(ivl_lpm_t net)
{
      unsigned width;
      const char*type = "";

      width = ivl_lpm_width(net);

      switch (ivl_lpm_type(net)) {
	  case IVL_LPM_CMP_GE:
	    type = "ge";
	    break;
	  case IVL_LPM_CMP_GT:
	    type = "gt";
	    break;
	  default:
	    assert(0);
      }

      fprintf(vvp_out, "L_%s .cmp/%s %u", 
	      vvp_mangle_id(ivl_lpm_name(net)), type, width);

      draw_lpm_arith_a_b_inputs(net);

      fprintf(vvp_out, ";\n");
}

/*
 * Draw == and != gates. This is done as XNOR functors to compare each
 * pair of bits. The result is combined with a wide and, or a NAND if
 * this is a NE.
 */
static void draw_lpm_eq(ivl_lpm_t net)
{
      unsigned width = ivl_lpm_width(net);
      unsigned idx;

      const char*and = ivl_lpm_type(net) == IVL_LPM_CMP_NE? "NAND" : "AND";

      ivl_nexus_t nex;

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    fprintf(vvp_out, "L_%s/L0C%u .functor XNOR, ",
		    vvp_mangle_id(ivl_lpm_name(net)), idx);

	    nex = ivl_lpm_data(net, idx);
	    draw_input_from_net(nex);

	    fprintf(vvp_out, ", ");

	    nex = ivl_lpm_datab(net, idx);
	    draw_input_from_net(nex);

	    fprintf(vvp_out, ", C<0>, C<0>;\n");
      }

      if (width <= 4) {
	    fprintf(vvp_out, "L_%s .functor %s",
		    vvp_mangle_id(ivl_lpm_name(net)), and);

	    for (idx = 0 ;  idx < width ;  idx += 1)
		  fprintf(vvp_out, ", L_%s/L0C%u",
			  vvp_mangle_id(ivl_lpm_name(net)), idx);

	    for (idx = width ;  idx < 4 ;  idx += 1)
		  fprintf(vvp_out, ", C<1>");

	    fprintf(vvp_out, ";\n");

      } else {
	    unsigned lwidth = width;
	    unsigned level = 1;
	    unsigned cnt;

	    unsigned bit;
	    unsigned first;
	    unsigned last;

	    cnt = (lwidth + 3) / 4;

	    while (cnt > 1) {
		  for (idx = 0 ;  idx < cnt ;  idx += 1) {
			first = idx*4;
			last = first + 4;
			if (last > lwidth)
			      last = lwidth;

			fprintf(vvp_out, "L_%s/L%uC%u .functor AND",
				vvp_mangle_id(ivl_lpm_name(net)),
				level, idx);

			for (bit = first ;  bit < last ;  bit += 1)
			      fprintf(vvp_out, ", L_%s/L%uC%u",
				      vvp_mangle_id(ivl_lpm_name(net)),
				      level-1, bit);

			for (bit = last ;  bit < (idx*4+4) ;  bit += 1)
			      fprintf(vvp_out, ", C<1>");

			fprintf(vvp_out, ";\n");
		  }

		  lwidth = cnt;
		  level += 1;
		  cnt = (lwidth + 3) / 4;
	    }

	    fprintf(vvp_out, "L_%s .functor %s",
		    vvp_mangle_id(ivl_lpm_name(net)), and);

	    for (idx = 0 ;  idx < lwidth ;  idx += 1)
		  fprintf(vvp_out, ", L_%s/L%uC%u",
			  vvp_mangle_id(ivl_lpm_name(net)),
			  level-1, idx);

	    for (idx = lwidth ;  idx < 4 ;  idx += 1)
		  fprintf(vvp_out, ", C<1>");

	    fprintf(vvp_out, ";\n");
      }
}

static void draw_lpm_ff(ivl_lpm_t net)
{
      unsigned width, idx;

      width = ivl_lpm_width(net);

      fprintf(vvp_out, "L_%s/def .udp/sequ \"DFF\", 3, 2,"
	      " \"?r100\","
	      " \"?r111\","
	      " \"?f1?-\","
	      " \"?\?1?-\","
	      " \"?*0\?-\";\n", vvp_mangle_id(ivl_lpm_name(net)));

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    ivl_nexus_t tmp;

	    fprintf(vvp_out, "L_%s/%u .udp ",
		    vvp_mangle_id(ivl_lpm_name(net)), idx);

	    fprintf(vvp_out, "L_%s/def, ", vvp_mangle_id(ivl_lpm_name(net)));

	    tmp = ivl_lpm_clk(net);
	    draw_input_from_net(tmp);

	    tmp = ivl_lpm_enable(net);
	    fprintf(vvp_out, ", ");
	    if (tmp)
		  draw_input_from_net(tmp);
	    else
		  fprintf(vvp_out, "C<1>");

	    tmp = ivl_lpm_data(net, idx);
	    assert(tmp);
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(tmp);

	    fprintf(vvp_out, ";\n");
      }
}

static void draw_lpm_shiftl(ivl_lpm_t net)
{
      unsigned idx, width, selects;

      width = ivl_lpm_width(net);
      selects = ivl_lpm_selects(net);

      if (ivl_lpm_type(net) == IVL_LPM_SHIFTR)
	    fprintf(vvp_out, "L_%s .shift/r %u", 
		    vvp_mangle_id(ivl_lpm_name(net)), width);
      else
	    fprintf(vvp_out, "L_%s .shift/l %u", 
		    vvp_mangle_id(ivl_lpm_name(net)), width);

      for (idx = 0 ;  idx < width ;  idx += 1) {
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(ivl_lpm_data(net, idx));
      }

      for (idx = 0 ;  idx < selects ;  idx += 1) {
	    fprintf(vvp_out, ", ");
	    draw_input_from_net(ivl_lpm_select(net, idx));
      }

      fprintf(vvp_out, ";\n");
}

static void draw_lpm_ufunc(ivl_lpm_t net)
{
      unsigned idx, bit;
      char comma;
      ivl_scope_t def = ivl_lpm_define(net);

      fprintf(vvp_out, "L_%s .ufunc TD_%s, %u",
	      vvp_mangle_id(ivl_lpm_name(net)),
	      ivl_scope_name(def),
	      ivl_lpm_width(net));

	/* Print all the net signals that connect to the input of the
	   function. Print them one per line, for convenience. */
      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1) {
	    comma = ' ';
	    fprintf(vvp_out, ",\n");
	    for (bit = 0 ; bit < ivl_lpm_data2_width(net, idx) ; bit += 1) {
		  fprintf(vvp_out, "%c ", comma);
		  draw_input_from_net(ivl_lpm_data2(net, idx, bit));
		  comma = ',';
	    }
      }


      assert((ivl_lpm_size(net)+1) == ivl_scope_ports(def));

	/* Now print all the variables in the function scope that
	   receive the input values given in the previous list. */
      for (idx = 0 ;  idx < ivl_lpm_size(net) ;  idx += 1) {
	    ivl_signal_t psig = ivl_scope_port(def, idx+1);
	    comma = ' ';

	    if (idx == 0)
		  fprintf(vvp_out, "\n(");
	    else
		  fprintf(vvp_out, ",\n");

	    for (bit = 0 ; bit < ivl_signal_pins(psig) ; bit += 1) {
		  fprintf(vvp_out, "%c V_%s[%u]", comma,
			  vvp_signal_label(psig), bit);
		  comma = ',';
	    }
      }

      fprintf(vvp_out, ")\n");

      { ivl_signal_t psig = ivl_scope_port(def, 0);
        assert(ivl_lpm_width(net) == ivl_signal_pins(psig));

	comma = ' ';
	for (idx = 0 ;  idx < ivl_lpm_width(net) ;  idx += 1) {
	      fprintf(vvp_out, "%c V_%s[%u]", comma,
		      vvp_signal_label(psig),
		      idx);
	      comma = ',';
	}
      }

      fprintf(vvp_out, ";\n");
}

static void draw_lpm_in_scope(ivl_lpm_t net)
{
      switch (ivl_lpm_type(net)) {

	  case IVL_LPM_RAM:
	    draw_lpm_ram(net);
	    return;

	  case IVL_LPM_ADD:
	  case IVL_LPM_SUB:
	  case IVL_LPM_MULT:
	  case IVL_LPM_DIVIDE:
	  case IVL_LPM_MOD:
	    draw_lpm_add(net);
	    return;

	  case IVL_LPM_CMP_EQ:
	  case IVL_LPM_CMP_NE:
	    draw_lpm_eq(net);
	    return;

	  case IVL_LPM_FF:
	    draw_lpm_ff(net);
	    return;

	  case IVL_LPM_CMP_GE:
	  case IVL_LPM_CMP_GT:
	    draw_lpm_cmp(net);
	    return;

	  case IVL_LPM_MUX:
	    draw_lpm_mux(net);
	    return;

	  case IVL_LPM_SHIFTL:
	  case IVL_LPM_SHIFTR:
	    draw_lpm_shiftl(net);
	    return;

	  case IVL_LPM_UFUNC:
	    draw_lpm_ufunc(net);
	    return;

	  default:
	    fprintf(stderr, "XXXX LPM not supported: %s\n",
		    ivl_lpm_name(net));
      }
}


static void draw_mem_in_scope(ivl_memory_t net)
{
      int root = ivl_memory_root(net);
      int last = root + ivl_memory_size(net) - 1;
      int msb = ivl_memory_width(net) - 1;
      int lsb = 0;
      fprintf(vvp_out, "M_%s .mem \"%s\", %u,%u, %u,%u;\n",
	      vvp_memory_label(net), 
	      vvp_mangle_name(ivl_memory_basename(net)),
	      msb, lsb, root, last);
}


int draw_scope(ivl_scope_t net, ivl_scope_t parent)
{
      unsigned idx;
      const char *type;
      switch (ivl_scope_type(net)) {
      case IVL_SCT_MODULE:   type = "module";   break;
      case IVL_SCT_FUNCTION: type = "function"; break;
      case IVL_SCT_TASK:     type = "task";     break;
      case IVL_SCT_BEGIN:    type = "begin";    break;
      case IVL_SCT_FORK:     type = "fork";     break;
      default:               type = "?";        assert(0);
      }

      fprintf(vvp_out, "S_%s .scope %s, \"%s\"",
	      vvp_mangle_id(ivl_scope_name(net)), 
	      type,
	      vvp_mangle_name(ivl_scope_name(net)));
      if (parent) {
	    fprintf(vvp_out, ", S_%s;\n",
		    vvp_mangle_id(ivl_scope_name(parent)));
      }
      else
	    fprintf(vvp_out, ";\n");
      
	/* Scan the scope for logic devices. For each device, draw out
	   a functor that connects pin 0 to the output, and the
	   remaining pins to inputs. */

      for (idx = 0 ;  idx < ivl_scope_logs(net) ;  idx += 1) {
	    ivl_net_logic_t lptr = ivl_scope_log(net, idx);
	    draw_logic_in_scope(lptr);
      }


	/* Scan the signals (reg and net) and draw the appropriate
	   statements to make the signal function. */

      for (idx = 0 ;  idx < ivl_scope_sigs(net) ;  idx += 1) {
	    ivl_signal_t sig = ivl_scope_sig(net, idx);

	    switch (ivl_signal_type(sig)) {
		case IVL_SIT_REG:
		  draw_reg_in_scope(sig);
		  break;
		default:
		  draw_net_in_scope(sig);
		  break;
	    }
      }

      for (idx = 0 ;  idx < ivl_scope_events(net) ;  idx += 1) {
	    ivl_event_t event = ivl_scope_event(net, idx);
	    draw_event_in_scope(event);
      }

      for (idx = 0 ;  idx < ivl_scope_mems(net) ;  idx += 1) {
	    ivl_memory_t mem = ivl_scope_mem(net, idx);
	    draw_mem_in_scope(mem);
      }

      for (idx = 0 ;  idx < ivl_scope_lpms(net) ;  idx += 1) {
	    ivl_lpm_t lpm = ivl_scope_lpm(net, idx);
	    draw_lpm_in_scope(lpm);
      }

      if (ivl_scope_type(net) == IVL_SCT_TASK)
	    draw_task_definition(net);

      if (ivl_scope_type(net) == IVL_SCT_FUNCTION)
	    draw_func_definition(net);

      ivl_scope_children(net, (ivl_scope_f*) draw_scope, net);
      return 0;
}

/*
 * $Log: vvp_scope.c,v $
 * Revision 1.78  2002/09/17 05:37:45  steve
 *  Generate vvp code for structural flip-flops.
 *
 * Revision 1.77  2002/08/12 01:35:04  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.76  2002/08/04 18:28:15  steve
 *  Do not use hierarchical names of memories to
 *  generate vvp labels. -tdll target does not
 *  used hierarchical name string to look up the
 *  memory objects in the design.
 *
 * Revision 1.75  2002/08/03 22:30:48  steve
 *  Eliminate use of ivl_signal_name for signal labels.
 *
 * Revision 1.74  2002/07/18 02:06:37  steve
 *  Need driver for sure in assign feedback and other cases.
 *
 * Revision 1.73  2002/07/08 04:04:07  steve
 *  Generate code for wide muxes.
 *
 * Revision 1.72  2002/07/05 21:26:17  steve
 *  Avoid emitting to vvp local net symbols.
 *
 * Revision 1.71  2002/06/21 04:59:35  steve
 *  Carry integerness throughout the compilation.
 *
 * Revision 1.70  2002/04/23 05:18:05  steve
 *  Tail size was wrong.
 *
 * Revision 1.69  2002/04/23 05:06:31  steve
 *  Handle bitsel muxes of odd shaped outputs.
 *
 * Revision 1.68  2002/04/23 03:53:59  steve
 *  Add support for non-constant bit select.
 *
 * Revision 1.67  2002/04/22 03:15:25  steve
 *  Keep delays applied to BUFZ devices.
 *
 * Revision 1.66  2002/03/18 00:18:50  steve
 *  Generate port information in the .ufunc statement.
 *
 * Revision 1.65  2002/03/09 02:10:22  steve
 *  Add the NetUserFunc netlist node.
 *
 * Revision 1.64  2002/01/12 17:49:41  steve
 *  Handle constants with drive strength z
 *
 * Revision 1.63  2002/01/12 04:03:40  steve
 *  Drive strengths for continuous assignments.
 *
 * Revision 1.62  2002/01/06 03:15:43  steve
 *  Constant values have drive strengths.
 *
 * Revision 1.61  2002/01/03 04:19:01  steve
 *  Add structural modulus support down to vvp.
 *
 * Revision 1.60  2001/12/15 02:13:33  steve
 *  Support all 3 TRI net types.
 *
 * Revision 1.59  2001/12/14 06:03:34  steve
 *  Generate notif functors.
 *
 * Revision 1.58  2001/12/14 02:05:13  steve
 *  Parse and handle drive strengths of gates to vvp.
 *
 * Revision 1.57  2001/12/06 03:31:24  steve
 *  Support functor delays for gates and UDP devices.
 *  (Stephan Boettcher)
 *
 * Revision 1.56  2001/11/01 04:26:57  steve
 *  Generate code for deassign and cassign.
 *
 * Revision 1.55  2001/10/24 03:43:45  steve
 *  Write resolvers before the .functor (PR#300)
 *
 * Revision 1.54  2001/10/22 02:04:37  steve
 *  unused idx warning.
 *
 * Revision 1.53  2001/10/22 00:04:51  steve
 *  Remove useless code for drawing .var inputs.
 *
 * Revision 1.52  2001/10/21 23:38:16  steve
 *  wrong variable for clk input to memory.
 *
 * Revision 1.51  2001/10/18 17:30:25  steve
 *  Support rnpmos devices. (Philip Blundell)
 *
 * Revision 1.50  2001/10/16 02:19:27  steve
 *  Support IVL_LPM_DIVIDE for structural divide.
 *
 * Revision 1.49  2001/10/15 02:58:27  steve
 *  Carry the type of the scope (Stephan Boettcher)
 *
 * Revision 1.48  2001/10/09 02:28:44  steve
 *  handle nmos and pmos devices.
 *
 * Revision 1.47  2001/09/15 18:27:04  steve
 *  Make configure detect malloc.h
 */

