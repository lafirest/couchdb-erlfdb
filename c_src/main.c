// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "erl_nif.h"
#include "fdb.h"

#include "atoms.h"
#include "resources.h"
#include "util.h"


// FoundationDB can only be intiailized once
// in a given OS process. By creating a random
// atom we can flag whether erlfdb was previously
// initialized in a given Erlang VM.
const char* SENTINEL = "faaffadf46e64b87bdd0fedbddd97b1a";


static int
erlfdb_load(ErlNifEnv* env, void** priv, ERL_NIF_TERM num_schedulers)
{
    erlfdb_init_atoms(env);

    if(!erlfdb_init_resources(env)) {
        return 1;
    }

    return 0;
}


static void
erlfdb_unload(ErlNifEnv* env, void* priv)
{
    // stop network and join
    return;
}


static ERL_NIF_TERM
erlfdb_can_initialize(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ERL_NIF_TERM atom;

    if(argc != 0) {
        return enif_make_badarg(env);
    }

    if(enif_make_existing_atom(env, SENTINEL, &atom, ERL_NIF_LATIN1)) {
        return ATOM_false;
    }

    enif_make_atom(env, SENTINEL);

    return ATOM_true;
}


static ERL_NIF_TERM
erlfdb_get_max_api_version(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    int vsn = fdb_get_max_api_version();

    if(argc != 0) {
        return enif_make_badarg(env);
    }

    return T2(env, ATOM_ok, enif_make_int(env, vsn));
}


static ERL_NIF_TERM
erlfdb_select_api_version(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    int vsn;
    fdb_error_t err;

    if(argc != 1) {
        return enif_make_badarg(env);
    }

    if(!enif_get_int(env, argv[0], &vsn)) {
        return enif_make_badarg(env);
    }

    err = fdb_select_api_version(vsn);

    if(err != 0) {
        return erlfdb_erlang_error(env, err);
    }

    return ATOM_ok;
}


static ERL_NIF_TERM
erlfdb_network_set_option(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    if(argc != 2) {
        return enif_make_badarg(env);
    }

    return ATOM_error;
}


static ERL_NIF_TERM
erlfdb_setup_network(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    fdb_error_t err;

    if(argc != 0) {
        return enif_make_badarg(env);
    }

    err = fdb_setup_network();
    if(err != 0) {
        return erlfdb_erlang_error(env, err);
    }

    // spawn network thread
    // wait for network thread

    return ATOM_ok;
}


#define NIF_FUNC(name, arity) {#name, arity, name}
static ErlNifFunc funcs[] =
{
    NIF_FUNC(erlfdb_can_initialize, 0),

    NIF_FUNC(erlfdb_get_max_api_version, 0),
    NIF_FUNC(erlfdb_select_api_version, 1),

    NIF_FUNC(erlfdb_network_set_option, 2),
    NIF_FUNC(erlfdb_setup_network, 0)
};
#undef NIF_FUNC


ERL_NIF_INIT(erlfdb_nif, funcs, &erlfdb_load, NULL, NULL, &erlfdb_unload);
