#ifndef RYML_SINGLE_HEADER
#include "c4/yml/std/std.hpp"
#include "c4/yml/parse.hpp"
#include "c4/yml/emit.hpp"
#include <c4/format.hpp>
#include <c4/yml/detail/checks.hpp>
#include <c4/yml/detail/print.hpp>
#endif

#include "./test_case.hpp"

#include <gtest/gtest.h>

#if defined(_MSC_VER)
#   pragma warning(push)
#   pragma warning(disable: 4389) // signed/unsigned mismatch
#elif defined(__clang__)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wdollar-in-identifier-extension"
#elif defined(__GNUC__)
#   pragma GCC diagnostic push
#endif

namespace c4 {
namespace yml {

//-------------------------------------------
Tree get_test_tree()
{
    Tree t = parse_in_arena("{a: b, c: d, e: [0, 1, 2, 3]}");
    // make sure the tree has strings in its arena
    NodeRef n = t.rootref();
    NodeRef ch = n.append_child();
    ch << key("serialized_key");
    ch << 89;
    return t;
}

//-------------------------------------------
TEST(Tree, copy_ctor)
{
    Tree t = get_test_tree();
    {
        Tree cp(t);
        test_invariants(t);
        test_invariants(cp);
        test_compare(t, cp);
        test_arena_not_shared(t, cp);
    }
}

//-------------------------------------------
TEST(Tree, move_ctor)
{
    Tree t = get_test_tree();
    Tree save(t);
    EXPECT_EQ(t.size(), save.size());

    {
        Tree cp(std::move(t));
        EXPECT_EQ(t.size(), 0);
        EXPECT_EQ(save.size(), cp.size());
        test_invariants(t);
        test_invariants(cp);
        test_invariants(save);
        test_compare(cp, save);
        test_arena_not_shared(t, cp);
        test_arena_not_shared(save, cp);
    }
}

//-------------------------------------------
TEST(Tree, copy_assign)
{
    Tree t = get_test_tree();
    Tree cp;

    cp = t;
    test_invariants(t);
    test_invariants(cp);
    test_compare(t, cp);
    test_arena_not_shared(t, cp);
}

//-------------------------------------------
TEST(Tree, move_assign)
{
    Tree t = get_test_tree();
    Tree cp;
    Tree save(t);
    EXPECT_EQ(t.size(), save.size());

    cp = std::move(t);
    test_invariants(t);
    test_invariants(cp);
    test_invariants(cp);
    test_invariants(save);
    test_compare(save, cp);
    test_arena_not_shared(t, cp);
    test_arena_not_shared(save, cp);
}

//-------------------------------------------
TEST(Tree, reserve)
{
    Tree t(16, 64);
    EXPECT_EQ(t.capacity(), 16);
    EXPECT_EQ(t.slack(), 15);
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.arena_capacity(), 64);
    EXPECT_EQ(t.arena_slack(), 64);
    EXPECT_EQ(t.arena_size(), 0);
    test_invariants(t);

    auto buf = t.m_buf;
    t.reserve(16);
    t.reserve_arena(64);
    EXPECT_EQ(t.m_buf, buf);
    EXPECT_EQ(t.capacity(), 16);
    EXPECT_EQ(t.slack(), 15);
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.arena_capacity(), 64);
    EXPECT_EQ(t.arena_slack(), 64);
    EXPECT_EQ(t.arena_size(), 0);
    test_invariants(t);

    t.reserve(32);
    t.reserve_arena(128);
    EXPECT_EQ(t.capacity(), 32);
    EXPECT_EQ(t.slack(), 31);
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.arena_capacity(), 128);
    EXPECT_EQ(t.arena_slack(), 128);
    EXPECT_EQ(t.arena_size(), 0);
    test_invariants(t);

    buf = t.m_buf;
    parse_in_arena("[a, b, c, d, e, f]", &t);
    EXPECT_EQ(t.m_buf, buf);
    EXPECT_EQ(t.capacity(), 32);
    EXPECT_EQ(t.slack(), 25);
    EXPECT_EQ(t.size(), 7);
    EXPECT_EQ(t.arena_capacity(), 128);
    EXPECT_EQ(t.arena_slack(), 110);
    EXPECT_EQ(t.arena_size(), 18);
    test_invariants(t);

    t.reserve(64);
    t.reserve_arena(256);
    EXPECT_EQ(t.capacity(), 64);
    EXPECT_EQ(t.slack(), 57);
    EXPECT_EQ(t.size(), 7);
    EXPECT_EQ(t.arena_capacity(), 256);
    EXPECT_EQ(t.arena_slack(), 238);
    EXPECT_EQ(t.arena_size(), 18);
    test_invariants(t);
}

TEST(Tree, clear)
{
    Tree t(16, 64);
    EXPECT_EQ(t.capacity(), 16);
    EXPECT_EQ(t.slack(), 15);
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.arena_capacity(), 64);
    EXPECT_EQ(t.arena_slack(), 64);
    EXPECT_EQ(t.arena_size(), 0);
    test_invariants(t);

    t.clear();
    t.clear_arena();
    EXPECT_EQ(t.capacity(), 16);
    EXPECT_EQ(t.slack(), 15);
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.arena_capacity(), 64);
    EXPECT_EQ(t.arena_slack(), 64);
    EXPECT_EQ(t.arena_size(), 0);
    test_invariants(t);

    auto buf = t.m_buf;
    t.reserve(16);
    t.reserve_arena(64);
    EXPECT_EQ(t.m_buf, buf);
    EXPECT_EQ(t.capacity(), 16);
    EXPECT_EQ(t.slack(), 15);
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.arena_capacity(), 64);
    EXPECT_EQ(t.arena_slack(), 64);
    EXPECT_EQ(t.arena_size(), 0);
    test_invariants(t);

    t.reserve(32);
    t.reserve_arena(128);
    EXPECT_EQ(t.capacity(), 32);
    EXPECT_EQ(t.slack(), 31);
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.arena_capacity(), 128);
    EXPECT_EQ(t.arena_slack(), 128);
    EXPECT_EQ(t.arena_size(), 0);
    test_invariants(t);

    buf = t.m_buf;
    parse_in_arena("[a, b, c, d, e, f]", &t);
    EXPECT_EQ(t.m_buf, buf);
    EXPECT_EQ(t.capacity(), 32);
    EXPECT_EQ(t.slack(), 25);
    EXPECT_EQ(t.size(), 7);
    EXPECT_EQ(t.arena_capacity(), 128);
    EXPECT_EQ(t.arena_slack(), 110);
    EXPECT_EQ(t.arena_size(), 18);
    test_invariants(t);

    t.clear();
    t.clear_arena();
    EXPECT_EQ(t.capacity(), 32);
    EXPECT_EQ(t.slack(), 31);
    EXPECT_EQ(t.size(), 1);
    EXPECT_EQ(t.arena_capacity(), 128);
    EXPECT_EQ(t.arena_slack(), 128);
    EXPECT_EQ(t.arena_size(), 0);
    test_invariants(t);
}


//-------------------------------------------

TEST(Tree, ref)
{
    Tree t = parse_in_arena("[0, 1, 2, 3]");
    EXPECT_EQ(t.ref(0).id(), 0);
    EXPECT_EQ(t.ref(1).id(), 1);
    EXPECT_EQ(t.ref(2).id(), 2);
    EXPECT_EQ(t.ref(3).id(), 3);
    EXPECT_EQ(t.ref(4).id(), 4);
    EXPECT_TRUE(t.ref(0).is_seq());
    EXPECT_TRUE(t.ref(1).is_val());
    EXPECT_TRUE(t.ref(2).is_val());
    EXPECT_TRUE(t.ref(3).is_val());
    EXPECT_TRUE(t.ref(4).is_val());
}

TEST(Tree, ref_const)
{
    const Tree t = parse_in_arena("[0, 1, 2, 3]");
    EXPECT_EQ(t.ref(0).id(), 0);
    EXPECT_EQ(t.ref(1).id(), 1);
    EXPECT_EQ(t.ref(2).id(), 2);
    EXPECT_EQ(t.ref(3).id(), 3);
    EXPECT_EQ(t.ref(4).id(), 4);
    EXPECT_TRUE(t.ref(0).is_seq());
    EXPECT_TRUE(t.ref(1).is_val());
    EXPECT_TRUE(t.ref(2).is_val());
    EXPECT_TRUE(t.ref(3).is_val());
    EXPECT_TRUE(t.ref(4).is_val());
}


TEST(Tree, operator_square_brackets)
{
    {
        Tree t = parse_in_arena("[0, 1, 2, 3, 4]");
        Tree &m = t;
        Tree const& cm = t;
        EXPECT_EQ(m[0].val(), "0");
        EXPECT_EQ(m[1].val(), "1");
        EXPECT_EQ(m[2].val(), "2");
        EXPECT_EQ(m[3].val(), "3");
        EXPECT_EQ(m[4].val(), "4");
        EXPECT_EQ(cm[0].val(), "0");
        EXPECT_EQ(cm[1].val(), "1");
        EXPECT_EQ(cm[2].val(), "2");
        EXPECT_EQ(cm[3].val(), "3");
        EXPECT_EQ(cm[4].val(), "4");
        //
        EXPECT_TRUE(m[0]  == "0");
        EXPECT_TRUE(m[1]  == "1");
        EXPECT_TRUE(m[2]  == "2");
        EXPECT_TRUE(m[3]  == "3");
        EXPECT_TRUE(m[4]  == "4");
        EXPECT_TRUE(cm[0] == "0");
        EXPECT_TRUE(cm[1] == "1");
        EXPECT_TRUE(cm[2] == "2");
        EXPECT_TRUE(cm[3] == "3");
        EXPECT_TRUE(cm[4] == "4");
        //
        EXPECT_FALSE(m[0]  != "0");
        EXPECT_FALSE(m[1]  != "1");
        EXPECT_FALSE(m[2]  != "2");
        EXPECT_FALSE(m[3]  != "3");
        EXPECT_FALSE(m[4]  != "4");
        EXPECT_FALSE(cm[0] != "0");
        EXPECT_FALSE(cm[1] != "1");
        EXPECT_FALSE(cm[2] != "2");
        EXPECT_FALSE(cm[3] != "3");
        EXPECT_FALSE(cm[4] != "4");
    }
    {
        Tree t = parse_in_arena("{a: 0, b: 1, c: 2, d: 3, e: 4}");
        Tree &m = t;
        Tree const& cm = t;
        EXPECT_EQ(m["a"].val(), "0");
        EXPECT_EQ(m["b"].val(), "1");
        EXPECT_EQ(m["c"].val(), "2");
        EXPECT_EQ(m["d"].val(), "3");
        EXPECT_EQ(m["e"].val(), "4");
        EXPECT_EQ(cm["a"].val(), "0");
        EXPECT_EQ(cm["b"].val(), "1");
        EXPECT_EQ(cm["c"].val(), "2");
        EXPECT_EQ(cm["d"].val(), "3");
        EXPECT_EQ(cm["e"].val(), "4");
        //
        EXPECT_TRUE(m["a"] == "0");
        EXPECT_TRUE(m["b"] == "1");
        EXPECT_TRUE(m["c"] == "2");
        EXPECT_TRUE(m["d"] == "3");
        EXPECT_TRUE(m["e"] == "4");
        EXPECT_TRUE(cm["a"] == "0");
        EXPECT_TRUE(cm["b"] == "1");
        EXPECT_TRUE(cm["c"] == "2");
        EXPECT_TRUE(cm["d"] == "3");
        EXPECT_TRUE(cm["e"] == "4");
        //
        EXPECT_FALSE(m["a"] != "0");
        EXPECT_FALSE(m["b"] != "1");
        EXPECT_FALSE(m["c"] != "2");
        EXPECT_FALSE(m["d"] != "3");
        EXPECT_FALSE(m["e"] != "4");
        EXPECT_FALSE(cm["a"] != "0");
        EXPECT_FALSE(cm["b"] != "1");
        EXPECT_FALSE(cm["c"] != "2");
        EXPECT_FALSE(cm["d"] != "3");
        EXPECT_FALSE(cm["e"] != "4");
    }
}

TEST(Tree, relocate)
{
    // create a tree with anchors and refs, and copy it to ensure the
    // relocation also applies to the anchors and refs. Ensure to put
    // the source in the arena so that it gets relocated.
    Tree tree = parse_in_arena(R"(&keyanchor key: val
key2: &valanchor val2
keyref: *keyanchor
*valanchor: was val anchor
!!int 0: !!str foo
!!str doe: !!str a deer a female deer
ray: a drop of golden sun
me: a name I call myself
far: a long long way to run
)");
    Tree copy = tree;
    EXPECT_EQ(copy.size(), tree.size());
    EXPECT_EQ(emitrs<std::string>(copy), R"(&keyanchor key: val
key2: &valanchor val2
keyref: *keyanchor
*valanchor: was val anchor
!!int 0: !!str foo
!!str doe: !!str a deer a female deer
ray: a drop of golden sun
me: a name I call myself
far: a long long way to run
)");
    //
    Tree copy2 = copy;
    EXPECT_EQ(copy.size(), tree.size());
    copy2.resolve();
    EXPECT_EQ(emitrs<std::string>(copy2), R"(key: val
key2: val2
keyref: key
val2: was val anchor
!!int 0: !!str foo
!!str doe: !!str a deer a female deer
ray: a drop of golden sun
me: a name I call myself
far: a long long way to run
)");
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void node_scalar_test_empty(NodeScalar const& s)
{
    EXPECT_TRUE(s.empty());
    EXPECT_EQ(s.tag, "");
    EXPECT_EQ(s.tag.len, 0u);
    EXPECT_TRUE(s.tag.empty());
    EXPECT_EQ(s.scalar, "");
    EXPECT_EQ(s.scalar.len, 0u);
    EXPECT_TRUE(s.scalar.empty());
}

void node_scalar_test_foo(NodeScalar const& s, bool with_tag=false)
{
    EXPECT_FALSE(s.empty());
    if(with_tag)
    {
        EXPECT_EQ(s.tag, "!!str");
        EXPECT_EQ(s.tag.len, 5u);
        EXPECT_FALSE(s.tag.empty());
    }
    else
    {
        EXPECT_EQ(s.tag, "");
        EXPECT_EQ(s.tag.len, 0u);
        EXPECT_TRUE(s.tag.empty());
    }
    EXPECT_EQ(s.scalar, "foo");
    EXPECT_EQ(s.scalar.len, 3u);
    EXPECT_FALSE(s.scalar.empty());
}

void node_scalar_test_foo3(NodeScalar const& s, bool with_tag=false)
{
    EXPECT_FALSE(s.empty());
    if(with_tag)
    {
        EXPECT_EQ(s.tag, "!!str+++");
        EXPECT_EQ(s.tag.len, 8u);
        EXPECT_FALSE(s.tag.empty());
    }
    else
    {
        EXPECT_EQ(s.tag, "");
        EXPECT_EQ(s.tag.len, 0u);
        EXPECT_TRUE(s.tag.empty());
    }
    EXPECT_EQ(s.scalar, "foo3");
    EXPECT_EQ(s.scalar.len, 4u);
    EXPECT_FALSE(s.scalar.empty());
}

TEST(NodeScalar, ctor_empty)
{
    NodeScalar s;
    node_scalar_test_empty(s);
}

TEST(NodeScalar, ctor__untagged)
{
    {
        const char sarr[] = "foo";
        const char *sptr = "foo";
        csubstr ssp = "foo";

        for(auto s : {NodeScalar(sarr), NodeScalar(to_csubstr(sptr)), NodeScalar(ssp)})
        {
            node_scalar_test_foo(s);
        }

        NodeScalar s;
        s = {sarr};
        node_scalar_test_foo(s);
        s = to_csubstr(sptr);
        node_scalar_test_foo(s);
        s = {ssp};
        node_scalar_test_foo(s);
    }

    {
        const char sarr[] = "foo3";
        const char *sptr = "foo3";
        csubstr ssp = "foo3";

        for(auto s : {NodeScalar(sarr), NodeScalar(to_csubstr(sptr)), NodeScalar(ssp)})
        {
            node_scalar_test_foo3(s);
        }

        NodeScalar s;
        {
            SCOPED_TRACE("here 1");
            s = {sarr};
            node_scalar_test_foo3(s);
        }
        {
            SCOPED_TRACE("here 2");
            s = to_csubstr(sptr);
            node_scalar_test_foo3(s);
        }
        {
            SCOPED_TRACE("here 3");
            s = ssp;
            node_scalar_test_foo3(s);
        }
    }
}

TEST(NodeScalar, ctor__tagged)
{
    {
        const char sarr[] = "foo", tarr[] = "!!str";
        const char *sptr = "foo";
        const char *tptr = "!!str";
        csubstr ssp = "foo", tsp = "!!str";

        for(NodeScalar s : {
                NodeScalar(tsp, ssp),
                    NodeScalar(tsp, to_csubstr(sptr)),
                    NodeScalar(tsp, sarr),
                NodeScalar(to_csubstr(tptr), ssp),
                    NodeScalar(to_csubstr(tptr), to_csubstr(sptr)),
                    NodeScalar(to_csubstr(tptr), sarr),
                NodeScalar(tarr, ssp),
                    NodeScalar(tarr, to_csubstr(sptr)),
                    NodeScalar(tarr, sarr),
        })
        {
            node_scalar_test_foo(s, true);
        }

        NodeScalar s;

        {
            SCOPED_TRACE("here 0.0");
            s = {tsp, ssp};
            node_scalar_test_foo(s, true);
        }
        {
            SCOPED_TRACE("here 0.1");
            s = {tsp, to_csubstr(sptr)};
            node_scalar_test_foo(s, true);
        }
        {
            SCOPED_TRACE("here 0.2");
            s = {tsp, sarr};
            node_scalar_test_foo(s, true);
        }

        {
            SCOPED_TRACE("here 1.0");
            s = {to_csubstr(tptr), ssp};
            node_scalar_test_foo(s, true);
        }
        {
            SCOPED_TRACE("here 1.1");
            s = {to_csubstr(tptr), to_csubstr(sptr)};
            node_scalar_test_foo(s, true);
        }
        {
            SCOPED_TRACE("here 1.3");
            s = {to_csubstr(tptr), sarr};
            node_scalar_test_foo(s, true);
        }

        {
            SCOPED_TRACE("here 3.0");
            s = {tarr, ssp};
            node_scalar_test_foo(s, true);
        }
        {
            SCOPED_TRACE("here 3.1");
            s = {tarr, to_csubstr(sptr)};
            node_scalar_test_foo(s, true);
        }
        {
            SCOPED_TRACE("here 3.3");
            s = {tarr, sarr};
            node_scalar_test_foo(s, true);
        }

    }

    {
        const char sarr[] = "foo3", tarr[] = "!!str+++";
        const char *sptr = "foo3";
        const char *tptr = "!!str+++";
        csubstr ssp = "foo3", tsp = "!!str+++";

        NodeScalar wtf = {tsp, ssp};
        EXPECT_EQ(wtf.tag, tsp);
        EXPECT_EQ(wtf.scalar, ssp);
        for(auto s : {
                NodeScalar(tsp, ssp),
                    NodeScalar(tsp, to_csubstr(sptr)),
                    NodeScalar(tsp, sarr),
                NodeScalar(to_csubstr(tptr), ssp),
                    NodeScalar(to_csubstr(tptr), to_csubstr(sptr)),
                    NodeScalar(to_csubstr(tptr), sarr),
                NodeScalar(tarr, ssp),
                    NodeScalar(tarr, to_csubstr(sptr)),
                    NodeScalar(tarr, sarr),
        })
        {
            node_scalar_test_foo3(s, true);
        }

        NodeScalar s;

        {
            SCOPED_TRACE("here 0.0");
            s = {tsp, ssp};
            node_scalar_test_foo3(s, true);
        }
        {
            SCOPED_TRACE("here 0.1");
            s = {tsp, to_csubstr(sptr)};
            node_scalar_test_foo3(s, true);
        }
        {
            SCOPED_TRACE("here 0.3");
            s = {tsp, sarr};
            node_scalar_test_foo3(s, true);
        }

        {
            SCOPED_TRACE("here 1.0");
            s = {to_csubstr(tptr), ssp};
            node_scalar_test_foo3(s, true);
        }
        {
            SCOPED_TRACE("here 1.1");
            s = {to_csubstr(tptr), to_csubstr(sptr)};
            node_scalar_test_foo3(s, true);
        }
        {
            SCOPED_TRACE("here 1.3");
            s = {to_csubstr(tptr), sarr};
            node_scalar_test_foo3(s, true);
        }

        {
            SCOPED_TRACE("here 3.0");
            s = {tarr, ssp};
            node_scalar_test_foo3(s, true);
        }
        {
            SCOPED_TRACE("here 3.1");
            s = {tarr, to_csubstr(sptr)};
            node_scalar_test_foo3(s, true);
        }
        {
            SCOPED_TRACE("here 3.3");
            s = {tarr, sarr};
            node_scalar_test_foo3(s, true);
        }

    }

}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TEST(NodeType, type_str)
{
    // avoid coverage misses
    EXPECT_EQ(to_csubstr(NodeType(KEYVAL).type_str()), "KEYVAL");
    EXPECT_EQ(to_csubstr(NodeType(KEY).type_str()), "KEY");
    EXPECT_EQ(to_csubstr(NodeType(VAL).type_str()), "VAL");
    EXPECT_EQ(to_csubstr(NodeType(MAP).type_str()), "MAP");
    EXPECT_EQ(to_csubstr(NodeType(SEQ).type_str()), "SEQ");
    EXPECT_EQ(to_csubstr(NodeType(KEYMAP).type_str()), "KEYMAP");
    EXPECT_EQ(to_csubstr(NodeType(KEYSEQ).type_str()), "KEYSEQ");
    EXPECT_EQ(to_csubstr(NodeType(DOCSEQ).type_str()), "DOCSEQ");
    EXPECT_EQ(to_csubstr(NodeType(DOCMAP).type_str()), "DOCMAP");
    EXPECT_EQ(to_csubstr(NodeType(DOCVAL).type_str()), "DOCVAL");
    EXPECT_EQ(to_csubstr(NodeType(DOC).type_str()), "DOC");
    EXPECT_EQ(to_csubstr(NodeType(STREAM).type_str()), "STREAM");
    EXPECT_EQ(to_csubstr(NodeType(NOTYPE).type_str()), "NOTYPE");
    EXPECT_EQ(to_csubstr(NodeType(KEYVAL|KEYREF).type_str()), "KEYVAL***");
    EXPECT_EQ(to_csubstr(NodeType(KEYVAL|VALREF).type_str()), "KEYVAL***");
    EXPECT_EQ(to_csubstr(NodeType(KEYVAL|KEYANCH).type_str()), "KEYVAL***");
    EXPECT_EQ(to_csubstr(NodeType(KEYVAL|VALANCH).type_str()), "KEYVAL***");
    EXPECT_EQ(to_csubstr(NodeType(KEYVAL|KEYREF|VALANCH).type_str()), "KEYVAL***");
    EXPECT_EQ(to_csubstr(NodeType(KEYVAL|KEYANCH|VALREF).type_str()), "KEYVAL***");
    EXPECT_EQ(to_csubstr(NodeType(KEYMAP|KEYREF).type_str()), "KEYMAP***");
    EXPECT_EQ(to_csubstr(NodeType(KEYMAP|VALREF).type_str()), "KEYMAP***");
    EXPECT_EQ(to_csubstr(NodeType(KEYMAP|KEYANCH).type_str()), "KEYMAP***");
    EXPECT_EQ(to_csubstr(NodeType(KEYMAP|VALANCH).type_str()), "KEYMAP***");
    EXPECT_EQ(to_csubstr(NodeType(KEYMAP|KEYREF|VALANCH).type_str()), "KEYMAP***");
    EXPECT_EQ(to_csubstr(NodeType(KEYMAP|KEYANCH|VALREF).type_str()), "KEYMAP***");
    EXPECT_EQ(to_csubstr(NodeType(KEYSEQ|KEYREF).type_str()), "KEYSEQ***");
    EXPECT_EQ(to_csubstr(NodeType(KEYSEQ|VALREF).type_str()), "KEYSEQ***");
    EXPECT_EQ(to_csubstr(NodeType(KEYSEQ|KEYANCH).type_str()), "KEYSEQ***");
    EXPECT_EQ(to_csubstr(NodeType(KEYSEQ|VALANCH).type_str()), "KEYSEQ***");
    EXPECT_EQ(to_csubstr(NodeType(KEYSEQ|KEYREF|VALANCH).type_str()), "KEYSEQ***");
    EXPECT_EQ(to_csubstr(NodeType(KEYSEQ|KEYANCH|VALREF).type_str()), "KEYSEQ***");
    EXPECT_EQ(to_csubstr(NodeType(DOCSEQ|VALANCH).type_str()), "DOCSEQ***");
    EXPECT_EQ(to_csubstr(NodeType(DOCSEQ|VALREF).type_str()), "DOCSEQ***");
    EXPECT_EQ(to_csubstr(NodeType(DOCMAP|VALANCH).type_str()), "DOCMAP***");
    EXPECT_EQ(to_csubstr(NodeType(DOCMAP|VALREF).type_str()), "DOCMAP***");
    EXPECT_EQ(to_csubstr(NodeType(DOCVAL|VALANCH).type_str()), "DOCVAL***");
    EXPECT_EQ(to_csubstr(NodeType(DOCVAL|VALREF).type_str()), "DOCVAL***");
    EXPECT_EQ(to_csubstr(NodeType(KEY|KEYREF).type_str()), "KEY***");
    EXPECT_EQ(to_csubstr(NodeType(KEY|KEYANCH).type_str()), "KEY***");
    EXPECT_EQ(to_csubstr(NodeType(VAL|VALREF).type_str()), "VAL***");
    EXPECT_EQ(to_csubstr(NodeType(VAL|VALANCH).type_str()), "VAL***");
    EXPECT_EQ(to_csubstr(NodeType(MAP|VALREF).type_str()), "MAP***");
    EXPECT_EQ(to_csubstr(NodeType(MAP|VALANCH).type_str()), "MAP***");
    EXPECT_EQ(to_csubstr(NodeType(SEQ|VALREF).type_str()), "SEQ***");
    EXPECT_EQ(to_csubstr(NodeType(SEQ|VALANCH).type_str()), "SEQ***");
    EXPECT_EQ(to_csubstr(NodeType(DOC|VALREF).type_str()), "DOC***");
    EXPECT_EQ(to_csubstr(NodeType(DOC|VALANCH).type_str()), "DOC***");
    EXPECT_EQ(to_csubstr(NodeType(KEYREF).type_str()), "(unk)");
    EXPECT_EQ(to_csubstr(NodeType(VALREF).type_str()), "(unk)");
    EXPECT_EQ(to_csubstr(NodeType(KEYANCH).type_str()), "(unk)");
    EXPECT_EQ(to_csubstr(NodeType(VALANCH).type_str()), "(unk)");
}

TEST(NodeType, is_stream)
{
    EXPECT_FALSE(NodeType(NOTYPE).is_stream());
    EXPECT_TRUE(NodeType(STREAM).is_stream());
}

TEST(Tree, is_stream)
{
    Tree t = parse_in_arena(R"(---
foo: bar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t keyval_id = t.first_child(doc_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef keyval = t.ref(keyval_id);
    EXPECT_TRUE(t.is_stream(stream_id));
    EXPECT_FALSE(t.is_stream(doc_id));
    EXPECT_FALSE(t.is_stream(keyval_id));
    EXPECT_TRUE(stream.is_stream());
    EXPECT_FALSE(doc.is_stream());
    EXPECT_FALSE(keyval.is_stream());
    EXPECT_EQ(t.is_stream(stream_id), t._p(stream_id)->m_type.is_stream());
    EXPECT_EQ(t.is_stream(doc_id), t._p(doc_id)->m_type.is_stream());
    EXPECT_EQ(t.is_stream(keyval_id), t._p(keyval_id)->m_type.is_stream());
    EXPECT_EQ(stream.is_stream(), stream.get()->m_type.is_stream());
    EXPECT_EQ(doc.is_stream(), doc.get()->m_type.is_stream());
    EXPECT_EQ(keyval.is_stream(), keyval.get()->m_type.is_stream());
}

TEST(NodeType, is_doc)
{
    EXPECT_FALSE(NodeType(NOTYPE).is_doc());
    EXPECT_TRUE(NodeType(DOC).is_doc());
}

TEST(Tree, is_doc)
{
    Tree t = parse_in_arena(R"(---
foo: bar
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t keyval_id = t.first_child(doc_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_FALSE(t.is_doc(stream_id));
    EXPECT_TRUE(t.is_doc(doc_id));
    EXPECT_FALSE(t.is_doc(keyval_id));
    EXPECT_TRUE(t.is_doc(docval_id));
    EXPECT_FALSE(stream.is_doc());
    EXPECT_TRUE(doc.is_doc());
    EXPECT_FALSE(keyval.is_doc());
    EXPECT_TRUE(docval.is_doc());
    EXPECT_EQ(t.is_doc(stream_id), t._p(stream_id)->m_type.is_doc());
    EXPECT_EQ(t.is_doc(doc_id), t._p(doc_id)->m_type.is_doc());
    EXPECT_EQ(t.is_doc(keyval_id), t._p(keyval_id)->m_type.is_doc());
    EXPECT_EQ(t.is_doc(docval_id), t._p(docval_id)->m_type.is_doc());
    EXPECT_EQ(stream.is_doc(), stream.get()->m_type.is_doc());
    EXPECT_EQ(doc.is_doc(), doc.get()->m_type.is_doc());
    EXPECT_EQ(keyval.is_doc(), keyval.get()->m_type.is_doc());
    EXPECT_EQ(docval.is_doc(), docval.get()->m_type.is_doc());
}

TEST(NodeType, is_container)
{
    EXPECT_FALSE(NodeType(NOTYPE).is_container());
    EXPECT_FALSE(NodeType(VAL).is_container());
    EXPECT_FALSE(NodeType(KEY).is_container());
    EXPECT_FALSE(NodeType(KEYVAL).is_container());
    EXPECT_TRUE(NodeType(MAP).is_container());
    EXPECT_TRUE(NodeType(SEQ).is_container());
    EXPECT_TRUE(NodeType(KEYMAP).is_container());
    EXPECT_TRUE(NodeType(KEYSEQ).is_container());
    EXPECT_TRUE(NodeType(DOCMAP).is_container());
    EXPECT_TRUE(NodeType(DOCSEQ).is_container());
}

TEST(Tree, is_container)
{
    Tree t = parse_in_arena(R"(---
map: {foo: bar}
seq: [foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_TRUE(t.is_container(stream_id));
    EXPECT_TRUE(t.is_container(doc_id));
    EXPECT_TRUE(t.is_container(map_id));
    EXPECT_FALSE(t.is_container(keyval_id));
    EXPECT_TRUE(t.is_container(seq_id));
    EXPECT_FALSE(t.is_container(val_id));
    EXPECT_FALSE(t.is_container(docval_id));
    EXPECT_TRUE(stream.is_container());
    EXPECT_TRUE(doc.is_container());
    EXPECT_TRUE(map.is_container());
    EXPECT_FALSE(keyval.is_container());
    EXPECT_TRUE(seq.is_container());
    EXPECT_FALSE(val.is_container());
    EXPECT_FALSE(docval.is_container());
    EXPECT_EQ(t.is_container(stream_id), t._p(stream_id)->m_type.is_container());
    EXPECT_EQ(t.is_container(doc_id), t._p(doc_id)->m_type.is_container());
    EXPECT_EQ(t.is_container(map_id), t._p(map_id)->m_type.is_container());
    EXPECT_EQ(t.is_container(keyval_id), t._p(keyval_id)->m_type.is_container());
    EXPECT_EQ(t.is_container(seq_id), t._p(seq_id)->m_type.is_container());
    EXPECT_EQ(t.is_container(val_id), t._p(val_id)->m_type.is_container());
    EXPECT_EQ(t.is_container(docval_id), t._p(docval_id)->m_type.is_container());
    EXPECT_EQ(stream.is_container(), stream.get()->m_type.is_container());
    EXPECT_EQ(doc.is_container(), doc.get()->m_type.is_container());
    EXPECT_EQ(map.is_container(), map.get()->m_type.is_container());
    EXPECT_EQ(keyval.is_container(), keyval.get()->m_type.is_container());
    EXPECT_EQ(seq.is_container(), seq.get()->m_type.is_container());
    EXPECT_EQ(val.is_container(), val.get()->m_type.is_container());
    EXPECT_EQ(docval.is_container(), docval.get()->m_type.is_container());
}

TEST(NodeType, is_map)
{
    EXPECT_FALSE(NodeType(NOTYPE).is_map());
    EXPECT_FALSE(NodeType(VAL).is_map());
    EXPECT_FALSE(NodeType(KEY).is_map());
    EXPECT_TRUE(NodeType(MAP).is_map());
    EXPECT_TRUE(NodeType(KEYMAP).is_map());
    EXPECT_FALSE(NodeType(SEQ).is_map());
    EXPECT_FALSE(NodeType(KEYSEQ).is_map());
}

TEST(Tree, is_map)
{
    Tree t = parse_in_arena(R"(---
map: {foo: bar}
seq: [foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_FALSE(t.is_map(stream_id));
    EXPECT_TRUE(t.is_map(doc_id));
    EXPECT_TRUE(t.is_map(map_id));
    EXPECT_FALSE(t.is_map(keyval_id));
    EXPECT_FALSE(t.is_map(seq_id));
    EXPECT_FALSE(t.is_map(val_id));
    EXPECT_FALSE(t.is_map(docval_id));
    EXPECT_FALSE(stream.is_map());
    EXPECT_TRUE(doc.is_map());
    EXPECT_TRUE(map.is_map());
    EXPECT_FALSE(keyval.is_map());
    EXPECT_FALSE(seq.is_map());
    EXPECT_FALSE(val.is_map());
    EXPECT_FALSE(docval.is_map());
    EXPECT_EQ(t.is_map(stream_id), t._p(stream_id)->m_type.is_map());
    EXPECT_EQ(t.is_map(doc_id), t._p(doc_id)->m_type.is_map());
    EXPECT_EQ(t.is_map(map_id), t._p(map_id)->m_type.is_map());
    EXPECT_EQ(t.is_map(keyval_id), t._p(keyval_id)->m_type.is_map());
    EXPECT_EQ(t.is_map(seq_id), t._p(seq_id)->m_type.is_map());
    EXPECT_EQ(t.is_map(val_id), t._p(val_id)->m_type.is_map());
    EXPECT_EQ(t.is_map(docval_id), t._p(docval_id)->m_type.is_map());
    EXPECT_EQ(stream.is_map(), stream.get()->m_type.is_map());
    EXPECT_EQ(doc.is_map(), doc.get()->m_type.is_map());
    EXPECT_EQ(map.is_map(), map.get()->m_type.is_map());
    EXPECT_EQ(keyval.is_map(), keyval.get()->m_type.is_map());
    EXPECT_EQ(seq.is_map(), seq.get()->m_type.is_map());
    EXPECT_EQ(val.is_map(), val.get()->m_type.is_map());
    EXPECT_EQ(docval.is_map(), docval.get()->m_type.is_map());
}

TEST(NodeType, is_seq)
{
    EXPECT_FALSE(NodeType(NOTYPE).is_seq());
    EXPECT_FALSE(NodeType(VAL).is_seq());
    EXPECT_FALSE(NodeType(KEY).is_seq());
    EXPECT_FALSE(NodeType(MAP).is_seq());
    EXPECT_FALSE(NodeType(KEYMAP).is_seq());
    EXPECT_TRUE(NodeType(SEQ).is_seq());
    EXPECT_TRUE(NodeType(KEYSEQ).is_seq());
}

TEST(Tree, is_seq)
{
    Tree t = parse_in_arena(R"(---
map: {foo: bar}
seq: [foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_TRUE(t.is_seq(stream_id));
    EXPECT_FALSE(t.is_seq(doc_id));
    EXPECT_FALSE(t.is_seq(map_id));
    EXPECT_FALSE(t.is_seq(keyval_id));
    EXPECT_TRUE(t.is_seq(seq_id));
    EXPECT_FALSE(t.is_seq(val_id));
    EXPECT_FALSE(t.is_seq(docval_id));
    EXPECT_TRUE(stream.is_seq());
    EXPECT_FALSE(doc.is_seq());
    EXPECT_FALSE(map.is_seq());
    EXPECT_FALSE(keyval.is_seq());
    EXPECT_TRUE(seq.is_seq());
    EXPECT_FALSE(val.is_seq());
    EXPECT_FALSE(docval.is_seq());
    EXPECT_EQ(t.is_seq(stream_id), t._p(stream_id)->m_type.is_seq());
    EXPECT_EQ(t.is_seq(doc_id), t._p(doc_id)->m_type.is_seq());
    EXPECT_EQ(t.is_seq(map_id), t._p(map_id)->m_type.is_seq());
    EXPECT_EQ(t.is_seq(keyval_id), t._p(keyval_id)->m_type.is_seq());
    EXPECT_EQ(t.is_seq(seq_id), t._p(seq_id)->m_type.is_seq());
    EXPECT_EQ(t.is_seq(val_id), t._p(val_id)->m_type.is_seq());
    EXPECT_EQ(t.is_seq(docval_id), t._p(docval_id)->m_type.is_seq());
    EXPECT_EQ(stream.is_seq(), stream.get()->m_type.is_seq());
    EXPECT_EQ(doc.is_seq(), doc.get()->m_type.is_seq());
    EXPECT_EQ(map.is_seq(), map.get()->m_type.is_seq());
    EXPECT_EQ(keyval.is_seq(), keyval.get()->m_type.is_seq());
    EXPECT_EQ(seq.is_seq(), seq.get()->m_type.is_seq());
    EXPECT_EQ(val.is_seq(), val.get()->m_type.is_seq());
    EXPECT_EQ(docval.is_seq(), docval.get()->m_type.is_seq());
}

TEST(NodeType, has_val)
{
    EXPECT_FALSE(NodeType(NOTYPE).has_val());
    EXPECT_FALSE(NodeType(KEY).has_val());
    EXPECT_TRUE(NodeType(VAL).has_val());
    EXPECT_TRUE(NodeType(DOCVAL).has_val());
    EXPECT_TRUE(NodeType(KEYVAL).has_val());
    EXPECT_FALSE(NodeType(KEYMAP).has_val());
    EXPECT_FALSE(NodeType(KEYSEQ).has_val());
}

TEST(Tree, has_val)
{
    Tree t = parse_in_arena(R"(---
map: {foo: bar}
seq: [foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_FALSE(t.has_val(stream_id));
    EXPECT_FALSE(t.has_val(doc_id));
    EXPECT_FALSE(t.has_val(map_id));
    EXPECT_TRUE(t.has_val(keyval_id));
    EXPECT_FALSE(t.has_val(seq_id));
    EXPECT_TRUE(t.has_val(val_id));
    EXPECT_TRUE(t.has_val(docval_id));
    EXPECT_FALSE(stream.has_val());
    EXPECT_FALSE(doc.has_val());
    EXPECT_FALSE(map.has_val());
    EXPECT_TRUE(keyval.has_val());
    EXPECT_FALSE(seq.has_val());
    EXPECT_TRUE(val.has_val());
    EXPECT_TRUE(docval.has_val());
    EXPECT_EQ(t.has_val(stream_id), t._p(stream_id)->m_type.has_val());
    EXPECT_EQ(t.has_val(doc_id), t._p(doc_id)->m_type.has_val());
    EXPECT_EQ(t.has_val(map_id), t._p(map_id)->m_type.has_val());
    EXPECT_EQ(t.has_val(keyval_id), t._p(keyval_id)->m_type.has_val());
    EXPECT_EQ(t.has_val(seq_id), t._p(seq_id)->m_type.has_val());
    EXPECT_EQ(t.has_val(val_id), t._p(val_id)->m_type.has_val());
    EXPECT_EQ(t.has_val(docval_id), t._p(docval_id)->m_type.has_val());
    EXPECT_EQ(stream.has_val(), stream.get()->m_type.has_val());
    EXPECT_EQ(doc.has_val(), doc.get()->m_type.has_val());
    EXPECT_EQ(map.has_val(), map.get()->m_type.has_val());
    EXPECT_EQ(keyval.has_val(), keyval.get()->m_type.has_val());
    EXPECT_EQ(seq.has_val(), seq.get()->m_type.has_val());
    EXPECT_EQ(val.has_val(), val.get()->m_type.has_val());
    EXPECT_EQ(docval.has_val(), docval.get()->m_type.has_val());
}

TEST(NodeType, is_val)
{
    EXPECT_FALSE(NodeType(NOTYPE).is_val());
    EXPECT_FALSE(NodeType(KEY).is_val());
    EXPECT_TRUE(NodeType(VAL).is_val());
    EXPECT_TRUE(NodeType(DOCVAL).is_val());
    EXPECT_FALSE(NodeType(KEYVAL).is_val());
    EXPECT_FALSE(NodeType(KEYMAP).is_val());
    EXPECT_FALSE(NodeType(KEYSEQ).is_val());
}

TEST(Tree, is_val)
{
    Tree t = parse_in_arena(R"(---
map: {foo: bar}
seq: [foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_FALSE(t.is_val(stream_id));
    EXPECT_FALSE(t.is_val(doc_id));
    EXPECT_FALSE(t.is_val(map_id));
    EXPECT_FALSE(t.is_val(keyval_id));
    EXPECT_FALSE(t.is_val(seq_id));
    EXPECT_TRUE(t.is_val(val_id));
    EXPECT_TRUE(t.is_val(docval_id));
    EXPECT_FALSE(stream.is_val());
    EXPECT_FALSE(doc.is_val());
    EXPECT_FALSE(map.is_val());
    EXPECT_FALSE(keyval.is_val());
    EXPECT_FALSE(seq.is_val());
    EXPECT_TRUE(val.is_val());
    EXPECT_TRUE(docval.is_val());
    EXPECT_EQ(t.is_val(stream_id), t._p(stream_id)->m_type.is_val());
    EXPECT_EQ(t.is_val(doc_id), t._p(doc_id)->m_type.is_val());
    EXPECT_EQ(t.is_val(map_id), t._p(map_id)->m_type.is_val());
    EXPECT_EQ(t.is_val(keyval_id), t._p(keyval_id)->m_type.is_val());
    EXPECT_EQ(t.is_val(seq_id), t._p(seq_id)->m_type.is_val());
    EXPECT_EQ(t.is_val(val_id), t._p(val_id)->m_type.is_val());
    EXPECT_EQ(t.is_val(docval_id), t._p(docval_id)->m_type.is_val());
    EXPECT_EQ(stream.is_val(), stream.get()->m_type.is_val());
    EXPECT_EQ(doc.is_val(), doc.get()->m_type.is_val());
    EXPECT_EQ(map.is_val(), map.get()->m_type.is_val());
    EXPECT_EQ(keyval.is_val(), keyval.get()->m_type.is_val());
    EXPECT_EQ(seq.is_val(), seq.get()->m_type.is_val());
    EXPECT_EQ(val.is_val(), val.get()->m_type.is_val());
    EXPECT_EQ(docval.is_val(), docval.get()->m_type.is_val());
}

TEST(NodeType, has_key)
{
    EXPECT_FALSE(NodeType(NOTYPE).has_key());
    EXPECT_TRUE(NodeType(KEY).has_key());
    EXPECT_FALSE(NodeType(VAL).has_key());
    EXPECT_TRUE(NodeType(KEYVAL).has_key());
    EXPECT_TRUE(NodeType(KEYMAP).has_key());
    EXPECT_TRUE(NodeType(KEYSEQ).has_key());
}

TEST(Tree, has_key)
{
    Tree t = parse_in_arena(R"(---
map: {foo: bar}
seq: [foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_FALSE(t.has_key(stream_id));
    EXPECT_FALSE(t.has_key(doc_id));
    EXPECT_TRUE(t.has_key(map_id));
    EXPECT_TRUE(t.has_key(keyval_id));
    EXPECT_TRUE(t.has_key(seq_id));
    EXPECT_FALSE(t.has_key(val_id));
    EXPECT_FALSE(t.has_key(docval_id));
    EXPECT_FALSE(stream.has_key());
    EXPECT_FALSE(doc.has_key());
    EXPECT_TRUE(map.has_key());
    EXPECT_TRUE(keyval.has_key());
    EXPECT_TRUE(seq.has_key());
    EXPECT_FALSE(val.has_key());
    EXPECT_FALSE(docval.has_key());
    EXPECT_EQ(t.has_key(stream_id), t._p(stream_id)->m_type.has_key());
    EXPECT_EQ(t.has_key(doc_id), t._p(doc_id)->m_type.has_key());
    EXPECT_EQ(t.has_key(map_id), t._p(map_id)->m_type.has_key());
    EXPECT_EQ(t.has_key(keyval_id), t._p(keyval_id)->m_type.has_key());
    EXPECT_EQ(t.has_key(seq_id), t._p(seq_id)->m_type.has_key());
    EXPECT_EQ(t.has_key(val_id), t._p(val_id)->m_type.has_key());
    EXPECT_EQ(t.has_key(docval_id), t._p(docval_id)->m_type.has_key());
    EXPECT_EQ(stream.has_key(), stream.get()->m_type.has_key());
    EXPECT_EQ(doc.has_key(), doc.get()->m_type.has_key());
    EXPECT_EQ(map.has_key(), map.get()->m_type.has_key());
    EXPECT_EQ(keyval.has_key(), keyval.get()->m_type.has_key());
    EXPECT_EQ(seq.has_key(), seq.get()->m_type.has_key());
    EXPECT_EQ(val.has_key(), val.get()->m_type.has_key());
    EXPECT_EQ(docval.has_key(), docval.get()->m_type.has_key());
}

TEST(NodeType, is_keyval)
{
    EXPECT_FALSE(NodeType(NOTYPE).is_keyval());
    EXPECT_FALSE(NodeType(KEY).is_keyval());
    EXPECT_FALSE(NodeType(VAL).is_keyval());
    EXPECT_TRUE(NodeType(KEYVAL).is_keyval());
    EXPECT_FALSE(NodeType(DOCVAL).is_keyval());
    EXPECT_FALSE(NodeType(KEYMAP).is_keyval());
    EXPECT_FALSE(NodeType(KEYSEQ).is_keyval());
}

TEST(Tree, is_keyval)
{
    Tree t = parse_in_arena(R"(---
map: {foo: bar}
seq: [foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_FALSE(t.is_keyval(stream_id));
    EXPECT_FALSE(t.is_keyval(doc_id));
    EXPECT_FALSE(t.is_keyval(map_id));
    EXPECT_TRUE(t.is_keyval(keyval_id));
    EXPECT_FALSE(t.is_keyval(seq_id));
    EXPECT_FALSE(t.is_keyval(val_id));
    EXPECT_FALSE(t.is_keyval(docval_id));
    EXPECT_FALSE(stream.is_keyval());
    EXPECT_FALSE(doc.is_keyval());
    EXPECT_FALSE(map.is_keyval());
    EXPECT_TRUE(keyval.is_keyval());
    EXPECT_FALSE(seq.is_keyval());
    EXPECT_FALSE(val.is_keyval());
    EXPECT_FALSE(docval.is_keyval());
    EXPECT_EQ(t.is_keyval(stream_id), t._p(stream_id)->m_type.is_keyval());
    EXPECT_EQ(t.is_keyval(doc_id), t._p(doc_id)->m_type.is_keyval());
    EXPECT_EQ(t.is_keyval(map_id), t._p(map_id)->m_type.is_keyval());
    EXPECT_EQ(t.is_keyval(keyval_id), t._p(keyval_id)->m_type.is_keyval());
    EXPECT_EQ(t.is_keyval(seq_id), t._p(seq_id)->m_type.is_keyval());
    EXPECT_EQ(t.is_keyval(val_id), t._p(val_id)->m_type.is_keyval());
    EXPECT_EQ(t.is_keyval(docval_id), t._p(docval_id)->m_type.is_keyval());
    EXPECT_EQ(stream.is_keyval(), stream.get()->m_type.is_keyval());
    EXPECT_EQ(doc.is_keyval(), doc.get()->m_type.is_keyval());
    EXPECT_EQ(map.is_keyval(), map.get()->m_type.is_keyval());
    EXPECT_EQ(keyval.is_keyval(), keyval.get()->m_type.is_keyval());
    EXPECT_EQ(seq.is_keyval(), seq.get()->m_type.is_keyval());
    EXPECT_EQ(val.is_keyval(), val.get()->m_type.is_keyval());
    EXPECT_EQ(docval.is_keyval(), docval.get()->m_type.is_keyval());
}

TEST(NodeType, has_key_tag)
{
    EXPECT_FALSE(NodeType().has_key_tag());
    EXPECT_FALSE(NodeType(KEYTAG).has_key_tag());
    EXPECT_TRUE(NodeType(KEY|KEYTAG).has_key_tag());
}

TEST(Tree, has_key_tag)
{
    Tree t = parse_in_arena(R"(--- !docmaptag
!maptag map: {!footag foo: bar, notag: none}
!seqtag seq: [!footag foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t keyvalnotag_id = t.last_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t valnotag_id = t.last_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef keyvalnotag = t.ref(keyvalnotag_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef valnotag = t.ref(val_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_FALSE(t.has_key_tag(stream_id));
    EXPECT_FALSE(t.has_key_tag(doc_id));
    EXPECT_TRUE(t.has_key_tag(map_id));
    EXPECT_TRUE(t.has_key_tag(keyval_id));
    EXPECT_FALSE(t.has_key_tag(keyvalnotag_id));
    EXPECT_TRUE(t.has_key_tag(seq_id));
    EXPECT_FALSE(t.has_key_tag(val_id));
    EXPECT_FALSE(t.has_key_tag(valnotag_id));
    EXPECT_FALSE(t.has_key_tag(docval_id));
    EXPECT_FALSE(stream.has_key_tag());
    EXPECT_FALSE(doc.has_key_tag());
    EXPECT_TRUE(map.has_key_tag());
    EXPECT_TRUE(keyval.has_key_tag());
    EXPECT_FALSE(keyvalnotag.has_key_tag());
    EXPECT_TRUE(seq.has_key_tag());
    EXPECT_FALSE(val.has_key_tag());
    EXPECT_FALSE(valnotag.has_key_tag());
    EXPECT_FALSE(docval.has_key_tag());
    EXPECT_EQ(t.has_key_tag(stream_id), t._p(stream_id)->m_type.has_key_tag());
    EXPECT_EQ(t.has_key_tag(doc_id), t._p(doc_id)->m_type.has_key_tag());
    EXPECT_EQ(t.has_key_tag(map_id), t._p(map_id)->m_type.has_key_tag());
    EXPECT_EQ(t.has_key_tag(keyval_id), t._p(keyval_id)->m_type.has_key_tag());
    EXPECT_EQ(t.has_key_tag(keyvalnotag_id), t._p(keyvalnotag_id)->m_type.has_key_tag());
    EXPECT_EQ(t.has_key_tag(seq_id), t._p(seq_id)->m_type.has_key_tag());
    EXPECT_EQ(t.has_key_tag(val_id), t._p(val_id)->m_type.has_key_tag());
    EXPECT_EQ(t.has_key_tag(valnotag_id), t._p(valnotag_id)->m_type.has_key_tag());
    EXPECT_EQ(t.has_key_tag(docval_id), t._p(docval_id)->m_type.has_key_tag());
    EXPECT_EQ(stream.has_key_tag(), stream.get()->m_type.has_key_tag());
    EXPECT_EQ(doc.has_key_tag(), doc.get()->m_type.has_key_tag());
    EXPECT_EQ(map.has_key_tag(), map.get()->m_type.has_key_tag());
    EXPECT_EQ(keyval.has_key_tag(), keyval.get()->m_type.has_key_tag());
    EXPECT_EQ(keyvalnotag.has_key_tag(), keyvalnotag.get()->m_type.has_key_tag());
    EXPECT_EQ(seq.has_key_tag(), seq.get()->m_type.has_key_tag());
    EXPECT_EQ(val.has_key_tag(), val.get()->m_type.has_key_tag());
    EXPECT_EQ(valnotag.has_key_tag(), valnotag.get()->m_type.has_key_tag());
    EXPECT_EQ(docval.has_key_tag(), docval.get()->m_type.has_key_tag());
}

TEST(NodeType, has_val_tag)
{
    EXPECT_FALSE(NodeType().has_val_tag());
    EXPECT_FALSE(NodeType(VALTAG).has_val_tag());
    EXPECT_TRUE(NodeType(VAL|VALTAG).has_val_tag());
}

TEST(Tree, has_val_tag)
{
    Tree t = parse_in_arena(R"(--- !docmaptag
map: !maptag {foo: !bartag bar, notag: none}
seq: !seqtag [!footag foo, bar]
---
a scalar
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t keyvalnotag_id = t.last_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t valnotag_id = t.last_child(seq_id);
    const size_t docval_id = t.last_child(stream_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef keyvalnotag = t.ref(keyvalnotag_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef valnotag = t.ref(valnotag_id);
    const NodeRef docval = t.ref(docval_id);
    EXPECT_FALSE(t.has_val_tag(stream_id));
    EXPECT_TRUE(t.has_val_tag(doc_id));
    EXPECT_TRUE(t.has_val_tag(map_id));
    EXPECT_TRUE(t.has_val_tag(keyval_id));
    EXPECT_FALSE(t.has_val_tag(keyvalnotag_id));
    EXPECT_TRUE(t.has_val_tag(seq_id));
    EXPECT_TRUE(t.has_val_tag(val_id));
    EXPECT_FALSE(t.has_val_tag(valnotag_id));
    EXPECT_FALSE(t.has_val_tag(docval_id));
    EXPECT_FALSE(stream.has_val_tag());
    EXPECT_TRUE(doc.has_val_tag());
    EXPECT_TRUE(map.has_val_tag());
    EXPECT_TRUE(keyval.has_val_tag());
    EXPECT_FALSE(keyvalnotag.has_val_tag());
    EXPECT_TRUE(seq.has_val_tag());
    EXPECT_TRUE(val.has_val_tag());
    EXPECT_FALSE(valnotag.has_val_tag());
    EXPECT_FALSE(docval.has_val_tag());
    EXPECT_EQ(t.has_val_tag(stream_id), t._p(stream_id)->m_type.has_val_tag());
    EXPECT_EQ(t.has_val_tag(doc_id), t._p(doc_id)->m_type.has_val_tag());
    EXPECT_EQ(t.has_val_tag(map_id), t._p(map_id)->m_type.has_val_tag());
    EXPECT_EQ(t.has_val_tag(keyval_id), t._p(keyval_id)->m_type.has_val_tag());
    EXPECT_EQ(t.has_val_tag(keyvalnotag_id), t._p(keyvalnotag_id)->m_type.has_val_tag());
    EXPECT_EQ(t.has_val_tag(seq_id), t._p(seq_id)->m_type.has_val_tag());
    EXPECT_EQ(t.has_val_tag(val_id), t._p(val_id)->m_type.has_val_tag());
    EXPECT_EQ(t.has_val_tag(valnotag_id), t._p(valnotag_id)->m_type.has_val_tag());
    EXPECT_EQ(t.has_val_tag(docval_id), t._p(docval_id)->m_type.has_val_tag());
    EXPECT_EQ(stream.has_val_tag(), stream.get()->m_type.has_val_tag());
    EXPECT_EQ(doc.has_val_tag(), doc.get()->m_type.has_val_tag());
    EXPECT_EQ(map.has_val_tag(), map.get()->m_type.has_val_tag());
    EXPECT_EQ(keyval.has_val_tag(), keyval.get()->m_type.has_val_tag());
    EXPECT_EQ(keyvalnotag.has_val_tag(), keyvalnotag.get()->m_type.has_val_tag());
    EXPECT_EQ(seq.has_val_tag(), seq.get()->m_type.has_val_tag());
    EXPECT_EQ(val.has_val_tag(), val.get()->m_type.has_val_tag());
    EXPECT_EQ(valnotag.has_val_tag(), valnotag.get()->m_type.has_val_tag());
    EXPECT_EQ(docval.has_val_tag(), docval.get()->m_type.has_val_tag());
}

TEST(NodeType, has_key_anchor)
{
    EXPECT_FALSE(NodeType().has_key_anchor());
    EXPECT_FALSE(NodeType(KEYANCH).has_key_anchor());
    EXPECT_TRUE(NodeType(KEY|KEYANCH).has_key_anchor());
}

TEST(Tree, has_key_anchor)
{
    Tree t = parse_in_arena(R"(--- &docanchor
&mapanchor map: {&keyvalanchor foo: bar, anchor: none}
&seqanchor seq: [&valanchor foo, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t keyvalnoanchor_id = t.last_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t valnoanchor_id = t.last_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef keyvalnoanchor = t.ref(keyvalnoanchor_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef valnoanchor = t.ref(valnoanchor_id);
    EXPECT_FALSE(t.has_key_anchor(stream_id));
    EXPECT_FALSE(t.has_key_anchor(doc_id));
    EXPECT_TRUE(t.has_key_anchor(map_id));
    EXPECT_TRUE(t.has_key_anchor(keyval_id));
    EXPECT_FALSE(t.has_key_anchor(keyvalnoanchor_id));
    EXPECT_TRUE(t.has_key_anchor(seq_id));
    EXPECT_FALSE(t.has_key_anchor(val_id));
    EXPECT_FALSE(t.has_key_anchor(valnoanchor_id));
    EXPECT_FALSE(stream.has_key_anchor());
    EXPECT_FALSE(doc.has_key_anchor());
    EXPECT_TRUE(map.has_key_anchor());
    EXPECT_TRUE(keyval.has_key_anchor());
    EXPECT_FALSE(keyvalnoanchor.has_key_anchor());
    EXPECT_TRUE(seq.has_key_anchor());
    EXPECT_FALSE(val.has_key_anchor());
    EXPECT_FALSE(valnoanchor.has_key_anchor());
    EXPECT_EQ(t.has_key_anchor(stream_id), t._p(stream_id)->m_type.has_key_anchor());
    EXPECT_EQ(t.has_key_anchor(doc_id), t._p(doc_id)->m_type.has_key_anchor());
    EXPECT_EQ(t.has_key_anchor(map_id), t._p(map_id)->m_type.has_key_anchor());
    EXPECT_EQ(t.has_key_anchor(keyval_id), t._p(keyval_id)->m_type.has_key_anchor());
    EXPECT_EQ(t.has_key_anchor(keyvalnoanchor_id), t._p(keyvalnoanchor_id)->m_type.has_key_anchor());
    EXPECT_EQ(t.has_key_anchor(seq_id), t._p(seq_id)->m_type.has_key_anchor());
    EXPECT_EQ(t.has_key_anchor(val_id), t._p(val_id)->m_type.has_key_anchor());
    EXPECT_EQ(t.has_key_anchor(valnoanchor_id), t._p(valnoanchor_id)->m_type.has_key_anchor());
    EXPECT_EQ(stream.has_key_anchor(), stream.get()->m_type.has_key_anchor());
    EXPECT_EQ(doc.has_key_anchor(), doc.get()->m_type.has_key_anchor());
    EXPECT_EQ(map.has_key_anchor(), map.get()->m_type.has_key_anchor());
    EXPECT_EQ(keyval.has_key_anchor(), keyval.get()->m_type.has_key_anchor());
    EXPECT_EQ(keyvalnoanchor.has_key_anchor(), keyvalnoanchor.get()->m_type.has_key_anchor());
    EXPECT_EQ(seq.has_key_anchor(), seq.get()->m_type.has_key_anchor());
    EXPECT_EQ(val.has_key_anchor(), val.get()->m_type.has_key_anchor());
    EXPECT_EQ(valnoanchor.has_key_anchor(), valnoanchor.get()->m_type.has_key_anchor());
}

TEST(NodeType, is_key_anchor)
{
    EXPECT_FALSE(NodeType().is_key_anchor());
    EXPECT_FALSE(NodeType(KEYANCH).is_key_anchor());
    EXPECT_TRUE(NodeType(KEY|KEYANCH).is_key_anchor());
}

TEST(Tree, is_key_anchor)
{
    Tree t = parse_in_arena(R"(--- &docanchor
&mapanchor map: {&keyvalanchor foo: bar, anchor: none}
&seqanchor seq: [&valanchor foo, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t keyvalnoanchor_id = t.last_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t valnoanchor_id = t.last_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef keyvalnoanchor = t.ref(keyvalnoanchor_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef valnoanchor = t.ref(valnoanchor_id);
    EXPECT_FALSE(t.is_key_anchor(stream_id));
    EXPECT_FALSE(t.is_key_anchor(doc_id));
    EXPECT_TRUE(t.is_key_anchor(map_id));
    EXPECT_TRUE(t.is_key_anchor(keyval_id));
    EXPECT_FALSE(t.is_key_anchor(keyvalnoanchor_id));
    EXPECT_TRUE(t.is_key_anchor(seq_id));
    EXPECT_FALSE(t.is_key_anchor(val_id));
    EXPECT_FALSE(t.is_key_anchor(valnoanchor_id));
    EXPECT_FALSE(stream.is_key_anchor());
    EXPECT_FALSE(doc.is_key_anchor());
    EXPECT_TRUE(map.is_key_anchor());
    EXPECT_TRUE(keyval.is_key_anchor());
    EXPECT_FALSE(keyvalnoanchor.is_key_anchor());
    EXPECT_TRUE(seq.is_key_anchor());
    EXPECT_FALSE(val.is_key_anchor());
    EXPECT_FALSE(valnoanchor.is_key_anchor());
    EXPECT_EQ(t.is_key_anchor(stream_id), t._p(stream_id)->m_type.is_key_anchor());
    EXPECT_EQ(t.is_key_anchor(doc_id), t._p(doc_id)->m_type.is_key_anchor());
    EXPECT_EQ(t.is_key_anchor(map_id), t._p(map_id)->m_type.is_key_anchor());
    EXPECT_EQ(t.is_key_anchor(keyval_id), t._p(keyval_id)->m_type.is_key_anchor());
    EXPECT_EQ(t.is_key_anchor(keyvalnoanchor_id), t._p(keyvalnoanchor_id)->m_type.is_key_anchor());
    EXPECT_EQ(t.is_key_anchor(seq_id), t._p(seq_id)->m_type.is_key_anchor());
    EXPECT_EQ(t.is_key_anchor(val_id), t._p(val_id)->m_type.is_key_anchor());
    EXPECT_EQ(t.is_key_anchor(valnoanchor_id), t._p(valnoanchor_id)->m_type.is_key_anchor());
    EXPECT_EQ(stream.is_key_anchor(), stream.get()->m_type.is_key_anchor());
    EXPECT_EQ(doc.is_key_anchor(), doc.get()->m_type.is_key_anchor());
    EXPECT_EQ(map.is_key_anchor(), map.get()->m_type.is_key_anchor());
    EXPECT_EQ(keyval.is_key_anchor(), keyval.get()->m_type.is_key_anchor());
    EXPECT_EQ(keyvalnoanchor.is_key_anchor(), keyvalnoanchor.get()->m_type.is_key_anchor());
    EXPECT_EQ(seq.is_key_anchor(), seq.get()->m_type.is_key_anchor());
    EXPECT_EQ(val.is_key_anchor(), val.get()->m_type.is_key_anchor());
    EXPECT_EQ(valnoanchor.is_key_anchor(), valnoanchor.get()->m_type.is_key_anchor());
}

TEST(NodeType, has_val_anchor)
{
    EXPECT_FALSE(NodeType().has_val_anchor());
    EXPECT_FALSE(NodeType(VALANCH).has_val_anchor());
    EXPECT_TRUE(NodeType(VAL|VALANCH).has_val_anchor());
}

TEST(Tree, has_val_anchor)
{
    Tree t = parse_in_arena(R"(--- &docanchor
map: &mapanchor {foo: &keyvalanchor bar, anchor: none}
seq: &seqanchor [&valanchor foo, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t keyvalnoanchor_id = t.last_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t valnoanchor_id = t.last_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef keyvalnoanchor = t.ref(keyvalnoanchor_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef valnoanchor = t.ref(valnoanchor_id);
    EXPECT_FALSE(t.has_val_anchor(stream_id));
    EXPECT_FALSE(t.has_val_anchor(doc_id));
    EXPECT_TRUE(t.has_val_anchor(map_id));
    EXPECT_TRUE(t.has_val_anchor(keyval_id));
    EXPECT_FALSE(t.has_val_anchor(keyvalnoanchor_id));
    EXPECT_TRUE(t.has_val_anchor(seq_id));
    EXPECT_TRUE(t.has_val_anchor(val_id));
    EXPECT_FALSE(t.has_val_anchor(valnoanchor_id));
    EXPECT_FALSE(stream.has_val_anchor());
    EXPECT_FALSE(doc.has_val_anchor());
    EXPECT_TRUE(map.has_val_anchor());
    EXPECT_TRUE(keyval.has_val_anchor());
    EXPECT_FALSE(keyvalnoanchor.has_val_anchor());
    EXPECT_TRUE(seq.has_val_anchor());
    EXPECT_TRUE(val.has_val_anchor());
    EXPECT_FALSE(valnoanchor.has_val_anchor());
    EXPECT_EQ(t.has_val_anchor(stream_id), t._p(stream_id)->m_type.has_val_anchor());
    EXPECT_EQ(t.has_val_anchor(doc_id), t._p(doc_id)->m_type.has_val_anchor());
    EXPECT_EQ(t.has_val_anchor(map_id), t._p(map_id)->m_type.has_val_anchor());
    EXPECT_EQ(t.has_val_anchor(keyval_id), t._p(keyval_id)->m_type.has_val_anchor());
    EXPECT_EQ(t.has_val_anchor(keyvalnoanchor_id), t._p(keyvalnoanchor_id)->m_type.has_val_anchor());
    EXPECT_EQ(t.has_val_anchor(seq_id), t._p(seq_id)->m_type.has_val_anchor());
    EXPECT_EQ(t.has_val_anchor(val_id), t._p(val_id)->m_type.has_val_anchor());
    EXPECT_EQ(t.has_val_anchor(valnoanchor_id), t._p(valnoanchor_id)->m_type.has_val_anchor());
    EXPECT_EQ(stream.has_val_anchor(), stream.get()->m_type.has_val_anchor());
    EXPECT_EQ(doc.has_val_anchor(), doc.get()->m_type.has_val_anchor());
    EXPECT_EQ(map.has_val_anchor(), map.get()->m_type.has_val_anchor());
    EXPECT_EQ(keyval.has_val_anchor(), keyval.get()->m_type.has_val_anchor());
    EXPECT_EQ(keyvalnoanchor.has_val_anchor(), keyvalnoanchor.get()->m_type.has_val_anchor());
    EXPECT_EQ(seq.has_val_anchor(), seq.get()->m_type.has_val_anchor());
    EXPECT_EQ(val.has_val_anchor(), val.get()->m_type.has_val_anchor());
    EXPECT_EQ(valnoanchor.has_val_anchor(), valnoanchor.get()->m_type.has_val_anchor());
}

TEST(NodeType, is_val_anchor)
{
    EXPECT_FALSE(NodeType().is_val_anchor());
    EXPECT_FALSE(NodeType(VALANCH).is_val_anchor());
    EXPECT_TRUE(NodeType(VAL|VALANCH).is_val_anchor());
}

TEST(Tree, is_val_anchor)
{
    Tree t = parse_in_arena(R"(--- &docanchor
map: &mapanchor {foo: &keyvalanchor bar, anchor: none}
seq: &seqanchor [&valanchor foo, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t keyvalnoanchor_id = t.last_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t valnoanchor_id = t.last_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef keyvalnoanchor = t.ref(keyvalnoanchor_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef valnoanchor = t.ref(valnoanchor_id);
    EXPECT_FALSE(t.is_val_anchor(stream_id));
    EXPECT_FALSE(t.is_val_anchor(doc_id));
    EXPECT_TRUE(t.is_val_anchor(map_id));
    EXPECT_TRUE(t.is_val_anchor(keyval_id));
    EXPECT_FALSE(t.is_val_anchor(keyvalnoanchor_id));
    EXPECT_TRUE(t.is_val_anchor(seq_id));
    EXPECT_TRUE(t.is_val_anchor(val_id));
    EXPECT_FALSE(t.is_val_anchor(valnoanchor_id));
    EXPECT_FALSE(stream.is_val_anchor());
    EXPECT_FALSE(doc.is_val_anchor());
    EXPECT_TRUE(map.is_val_anchor());
    EXPECT_TRUE(keyval.is_val_anchor());
    EXPECT_FALSE(keyvalnoanchor.is_val_anchor());
    EXPECT_TRUE(seq.is_val_anchor());
    EXPECT_TRUE(val.is_val_anchor());
    EXPECT_FALSE(valnoanchor.is_val_anchor());
    EXPECT_EQ(t.is_val_anchor(stream_id), t._p(stream_id)->m_type.is_val_anchor());
    EXPECT_EQ(t.is_val_anchor(doc_id), t._p(doc_id)->m_type.is_val_anchor());
    EXPECT_EQ(t.is_val_anchor(map_id), t._p(map_id)->m_type.is_val_anchor());
    EXPECT_EQ(t.is_val_anchor(keyval_id), t._p(keyval_id)->m_type.is_val_anchor());
    EXPECT_EQ(t.is_val_anchor(keyvalnoanchor_id), t._p(keyvalnoanchor_id)->m_type.is_val_anchor());
    EXPECT_EQ(t.is_val_anchor(seq_id), t._p(seq_id)->m_type.is_val_anchor());
    EXPECT_EQ(t.is_val_anchor(val_id), t._p(val_id)->m_type.is_val_anchor());
    EXPECT_EQ(t.is_val_anchor(valnoanchor_id), t._p(valnoanchor_id)->m_type.is_val_anchor());
    EXPECT_EQ(stream.is_val_anchor(), stream.get()->m_type.is_val_anchor());
    EXPECT_EQ(doc.is_val_anchor(), doc.get()->m_type.is_val_anchor());
    EXPECT_EQ(map.is_val_anchor(), map.get()->m_type.is_val_anchor());
    EXPECT_EQ(keyval.is_val_anchor(), keyval.get()->m_type.is_val_anchor());
    EXPECT_EQ(keyvalnoanchor.is_val_anchor(), keyvalnoanchor.get()->m_type.is_val_anchor());
    EXPECT_EQ(seq.is_val_anchor(), seq.get()->m_type.is_val_anchor());
    EXPECT_EQ(val.is_val_anchor(), val.get()->m_type.is_val_anchor());
    EXPECT_EQ(valnoanchor.is_val_anchor(), valnoanchor.get()->m_type.is_val_anchor());
}

TEST(NodeType, has_anchor)
{
    EXPECT_FALSE(NodeType().has_anchor());
    EXPECT_TRUE(NodeType(VALANCH).has_anchor());
    EXPECT_TRUE(NodeType(KEYANCH).has_anchor());
    EXPECT_TRUE(NodeType(KEYANCH|VALANCH).has_anchor());
    EXPECT_TRUE(NodeType(KEY|VALANCH).has_anchor());
    EXPECT_TRUE(NodeType(VAL|KEYANCH).has_anchor());
    EXPECT_TRUE(NodeType(KEY|KEYANCH).has_anchor());
    EXPECT_TRUE(NodeType(VAL|VALANCH).has_anchor());
}

TEST(Tree, has_anchor)
{
    Tree t = parse_in_arena(R"(--- &docanchor
map: &mapanchor {foo: &keyvalanchor bar, anchor: none}
&seqanchor seq: [&valanchor foo, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t keyvalnoanchor_id = t.last_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t valnoanchor_id = t.last_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef keyvalnoanchor = t.ref(keyvalnoanchor_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef valnoanchor = t.ref(valnoanchor_id);
    EXPECT_FALSE(t.has_anchor(stream_id));
    EXPECT_FALSE(t.has_anchor(doc_id));
    EXPECT_TRUE(t.has_anchor(map_id));
    EXPECT_TRUE(t.has_anchor(keyval_id));
    EXPECT_FALSE(t.has_anchor(keyvalnoanchor_id));
    EXPECT_TRUE(t.has_anchor(seq_id));
    EXPECT_TRUE(t.has_anchor(val_id));
    EXPECT_FALSE(t.has_anchor(valnoanchor_id));
    EXPECT_FALSE(stream.has_anchor());
    EXPECT_FALSE(doc.has_anchor());
    EXPECT_TRUE(map.has_anchor());
    EXPECT_TRUE(keyval.has_anchor());
    EXPECT_FALSE(keyvalnoanchor.has_anchor());
    EXPECT_TRUE(seq.has_anchor());
    EXPECT_TRUE(val.has_anchor());
    EXPECT_FALSE(valnoanchor.has_anchor());
    EXPECT_EQ(t.has_anchor(stream_id), t._p(stream_id)->m_type.has_anchor());
    EXPECT_EQ(t.has_anchor(doc_id), t._p(doc_id)->m_type.has_anchor());
    EXPECT_EQ(t.has_anchor(map_id), t._p(map_id)->m_type.has_anchor());
    EXPECT_EQ(t.has_anchor(keyval_id), t._p(keyval_id)->m_type.has_anchor());
    EXPECT_EQ(t.has_anchor(keyvalnoanchor_id), t._p(keyvalnoanchor_id)->m_type.has_anchor());
    EXPECT_EQ(t.has_anchor(seq_id), t._p(seq_id)->m_type.has_anchor());
    EXPECT_EQ(t.has_anchor(val_id), t._p(val_id)->m_type.has_anchor());
    EXPECT_EQ(t.has_anchor(valnoanchor_id), t._p(valnoanchor_id)->m_type.has_anchor());
    EXPECT_EQ(stream.has_anchor(), stream.get()->m_type.has_anchor());
    EXPECT_EQ(doc.has_anchor(), doc.get()->m_type.has_anchor());
    EXPECT_EQ(map.has_anchor(), map.get()->m_type.has_anchor());
    EXPECT_EQ(keyval.has_anchor(), keyval.get()->m_type.has_anchor());
    EXPECT_EQ(keyvalnoanchor.has_anchor(), keyvalnoanchor.get()->m_type.has_anchor());
    EXPECT_EQ(seq.has_anchor(), seq.get()->m_type.has_anchor());
    EXPECT_EQ(val.has_anchor(), val.get()->m_type.has_anchor());
    EXPECT_EQ(valnoanchor.has_anchor(), valnoanchor.get()->m_type.has_anchor());
}

TEST(NodeType, is_anchor)
{
    EXPECT_FALSE(NodeType().is_anchor());
    EXPECT_TRUE(NodeType(VALANCH).is_anchor());
    EXPECT_TRUE(NodeType(KEYANCH).is_anchor());
    EXPECT_TRUE(NodeType(KEYANCH|VALANCH).is_anchor());
    EXPECT_TRUE(NodeType(KEY|VALANCH).is_anchor());
    EXPECT_TRUE(NodeType(VAL|KEYANCH).is_anchor());
    EXPECT_TRUE(NodeType(KEY|KEYANCH).is_anchor());
    EXPECT_TRUE(NodeType(VAL|VALANCH).is_anchor());
}

TEST(Tree, is_anchor)
{
    Tree t = parse_in_arena(R"(--- &docanchor
map: &mapanchor {foo: &keyvalanchor bar, anchor: none}
&seqanchor seq: [&valanchor foo, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t keyvalnoanchor_id = t.last_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const size_t valnoanchor_id = t.last_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef keyvalnoanchor = t.ref(keyvalnoanchor_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    const NodeRef valnoanchor = t.ref(valnoanchor_id);
    EXPECT_FALSE(t.is_anchor(stream_id));
    EXPECT_FALSE(t.is_anchor(doc_id));
    EXPECT_TRUE(t.is_anchor(map_id));
    EXPECT_TRUE(t.is_anchor(keyval_id));
    EXPECT_FALSE(t.is_anchor(keyvalnoanchor_id));
    EXPECT_TRUE(t.is_anchor(seq_id));
    EXPECT_TRUE(t.is_anchor(val_id));
    EXPECT_FALSE(t.is_anchor(valnoanchor_id));
    EXPECT_FALSE(stream.is_anchor());
    EXPECT_FALSE(doc.is_anchor());
    EXPECT_TRUE(map.is_anchor());
    EXPECT_TRUE(keyval.is_anchor());
    EXPECT_FALSE(keyvalnoanchor.is_anchor());
    EXPECT_TRUE(seq.is_anchor());
    EXPECT_TRUE(val.is_anchor());
    EXPECT_FALSE(valnoanchor.is_anchor());
    EXPECT_EQ(t.is_anchor(stream_id), t._p(stream_id)->m_type.is_anchor());
    EXPECT_EQ(t.is_anchor(doc_id), t._p(doc_id)->m_type.is_anchor());
    EXPECT_EQ(t.is_anchor(map_id), t._p(map_id)->m_type.is_anchor());
    EXPECT_EQ(t.is_anchor(keyval_id), t._p(keyval_id)->m_type.is_anchor());
    EXPECT_EQ(t.is_anchor(keyvalnoanchor_id), t._p(keyvalnoanchor_id)->m_type.is_anchor());
    EXPECT_EQ(t.is_anchor(seq_id), t._p(seq_id)->m_type.is_anchor());
    EXPECT_EQ(t.is_anchor(val_id), t._p(val_id)->m_type.is_anchor());
    EXPECT_EQ(t.is_anchor(valnoanchor_id), t._p(valnoanchor_id)->m_type.is_anchor());
    EXPECT_EQ(stream.is_anchor(), stream.get()->m_type.is_anchor());
    EXPECT_EQ(doc.is_anchor(), doc.get()->m_type.is_anchor());
    EXPECT_EQ(map.is_anchor(), map.get()->m_type.is_anchor());
    EXPECT_EQ(keyval.is_anchor(), keyval.get()->m_type.is_anchor());
    EXPECT_EQ(keyvalnoanchor.is_anchor(), keyvalnoanchor.get()->m_type.is_anchor());
    EXPECT_EQ(seq.is_anchor(), seq.get()->m_type.is_anchor());
    EXPECT_EQ(val.is_anchor(), val.get()->m_type.is_anchor());
    EXPECT_EQ(valnoanchor.is_anchor(), valnoanchor.get()->m_type.is_anchor());
}

TEST(NodeType, is_key_ref)
{
    EXPECT_FALSE(NodeType().is_key_ref());
    EXPECT_TRUE(NodeType(KEYREF).is_key_ref());
    EXPECT_TRUE(NodeType(KEY|KEYREF).is_key_ref());
}

TEST(Tree, is_key_ref)
{
    Tree t = parse_in_arena(R"(---
*mapref: {foo: bar, notag: none}
*seqref: [foo, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    EXPECT_FALSE(t.is_key_ref(stream_id));
    EXPECT_FALSE(t.is_key_ref(doc_id));
    EXPECT_TRUE(t.is_key_ref(map_id));
    EXPECT_FALSE(t.is_key_ref(keyval_id));
    EXPECT_TRUE(t.is_key_ref(seq_id));
    EXPECT_FALSE(t.is_key_ref(val_id));
    EXPECT_FALSE(stream.is_key_ref());
    EXPECT_FALSE(doc.is_key_ref());
    EXPECT_TRUE(map.is_key_ref());
    EXPECT_FALSE(keyval.is_key_ref());
    EXPECT_TRUE(seq.is_key_ref());
    EXPECT_FALSE(val.is_key_ref());
    EXPECT_EQ(t.is_key_ref(stream_id), t._p(stream_id)->m_type.is_key_ref());
    EXPECT_EQ(t.is_key_ref(doc_id), t._p(doc_id)->m_type.is_key_ref());
    EXPECT_EQ(t.is_key_ref(map_id), t._p(map_id)->m_type.is_key_ref());
    EXPECT_EQ(t.is_key_ref(keyval_id), t._p(keyval_id)->m_type.is_key_ref());
    EXPECT_EQ(t.is_key_ref(seq_id), t._p(seq_id)->m_type.is_key_ref());
    EXPECT_EQ(t.is_key_ref(val_id), t._p(val_id)->m_type.is_key_ref());
    EXPECT_EQ(stream.is_key_ref(), stream.get()->m_type.is_key_ref());
    EXPECT_EQ(doc.is_key_ref(), doc.get()->m_type.is_key_ref());
    EXPECT_EQ(map.is_key_ref(), map.get()->m_type.is_key_ref());
    EXPECT_EQ(keyval.is_key_ref(), keyval.get()->m_type.is_key_ref());
    EXPECT_EQ(seq.is_key_ref(), seq.get()->m_type.is_key_ref());
    EXPECT_EQ(val.is_key_ref(), val.get()->m_type.is_key_ref());
}

TEST(NodeType, is_val_ref)
{
    EXPECT_FALSE(NodeType().is_val_ref());
    EXPECT_TRUE(NodeType(VALREF).is_val_ref());
    EXPECT_TRUE(NodeType(VAL|VALREF).is_val_ref());
}

TEST(Tree, is_val_ref)
{
    Tree t = parse_in_arena(R"(---
map: {foo: *keyvalref, notag: none}
seq: [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    EXPECT_FALSE(t.is_val_ref(stream_id));
    EXPECT_FALSE(t.is_val_ref(doc_id));
    EXPECT_FALSE(t.is_val_ref(map_id));
    EXPECT_TRUE(t.is_val_ref(keyval_id));
    EXPECT_FALSE(t.is_val_ref(seq_id));
    EXPECT_TRUE(t.is_val_ref(val_id));
    EXPECT_FALSE(stream.is_val_ref());
    EXPECT_FALSE(doc.is_val_ref());
    EXPECT_FALSE(map.is_val_ref());
    EXPECT_TRUE(keyval.is_val_ref());
    EXPECT_FALSE(seq.is_val_ref());
    EXPECT_TRUE(val.is_val_ref());
    EXPECT_EQ(t.is_val_ref(stream_id), t._p(stream_id)->m_type.is_val_ref());
    EXPECT_EQ(t.is_val_ref(doc_id), t._p(doc_id)->m_type.is_val_ref());
    EXPECT_EQ(t.is_val_ref(map_id), t._p(map_id)->m_type.is_val_ref());
    EXPECT_EQ(t.is_val_ref(keyval_id), t._p(keyval_id)->m_type.is_val_ref());
    EXPECT_EQ(t.is_val_ref(seq_id), t._p(seq_id)->m_type.is_val_ref());
    EXPECT_EQ(t.is_val_ref(val_id), t._p(val_id)->m_type.is_val_ref());
    EXPECT_EQ(stream.is_val_ref(), stream.get()->m_type.is_val_ref());
    EXPECT_EQ(doc.is_val_ref(), doc.get()->m_type.is_val_ref());
    EXPECT_EQ(map.is_val_ref(), map.get()->m_type.is_val_ref());
    EXPECT_EQ(keyval.is_val_ref(), keyval.get()->m_type.is_val_ref());
    EXPECT_EQ(seq.is_val_ref(), seq.get()->m_type.is_val_ref());
    EXPECT_EQ(val.is_val_ref(), val.get()->m_type.is_val_ref());
}

TEST(NodeType, is_ref)
{
    EXPECT_FALSE(NodeType().is_ref());
    EXPECT_FALSE(NodeType(KEYVAL).is_ref());
    EXPECT_TRUE(NodeType(KEYREF).is_ref());
    EXPECT_TRUE(NodeType(VALREF).is_ref());
    EXPECT_TRUE(NodeType(KEY|VALREF).is_ref());
    EXPECT_TRUE(NodeType(VAL|KEYREF).is_ref());
    EXPECT_TRUE(NodeType(KEYREF|VALREF).is_ref());
}

TEST(Tree, is_ref)
{
    Tree t = parse_in_arena(R"(---
map: {foo: *keyvalref, notag: none}
seq: [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    EXPECT_FALSE(t.is_ref(stream_id));
    EXPECT_FALSE(t.is_ref(doc_id));
    EXPECT_FALSE(t.is_ref(map_id));
    EXPECT_TRUE(t.is_ref(keyval_id));
    EXPECT_FALSE(t.is_ref(seq_id));
    EXPECT_TRUE(t.is_ref(val_id));
    EXPECT_FALSE(stream.is_ref());
    EXPECT_FALSE(doc.is_ref());
    EXPECT_FALSE(map.is_ref());
    EXPECT_TRUE(keyval.is_ref());
    EXPECT_FALSE(seq.is_ref());
    EXPECT_TRUE(val.is_ref());
    EXPECT_EQ(t.is_ref(stream_id), t._p(stream_id)->m_type.is_ref());
    EXPECT_EQ(t.is_ref(doc_id), t._p(doc_id)->m_type.is_ref());
    EXPECT_EQ(t.is_ref(map_id), t._p(map_id)->m_type.is_ref());
    EXPECT_EQ(t.is_ref(keyval_id), t._p(keyval_id)->m_type.is_ref());
    EXPECT_EQ(t.is_ref(seq_id), t._p(seq_id)->m_type.is_ref());
    EXPECT_EQ(t.is_ref(val_id), t._p(val_id)->m_type.is_ref());
    EXPECT_EQ(stream.is_ref(), stream.get()->m_type.is_ref());
    EXPECT_EQ(doc.is_ref(), doc.get()->m_type.is_ref());
    EXPECT_EQ(map.is_ref(), map.get()->m_type.is_ref());
    EXPECT_EQ(keyval.is_ref(), keyval.get()->m_type.is_ref());
    EXPECT_EQ(seq.is_ref(), seq.get()->m_type.is_ref());
    EXPECT_EQ(val.is_ref(), val.get()->m_type.is_ref());
}

TEST(NodeType, is_anchor_or_ref)
{
    EXPECT_FALSE(NodeType().is_anchor_or_ref());
    EXPECT_FALSE(NodeType(KEYVAL).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(KEYREF).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(KEYANCH).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(VALREF).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(VALANCH).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(KEY|VALREF).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(KEY|VALANCH).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(VAL|KEYREF).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(VAL|VALANCH).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(KEY|VALANCH).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(KEYREF|VALREF).is_anchor_or_ref());
    EXPECT_TRUE(NodeType(KEYANCH|VALANCH).is_anchor_or_ref());
}

TEST(Tree, is_anchor_or_ref)
{
    Tree t = parse_in_arena(R"(---
&map map: {foo: *keyvalref, notag: none}
seq: &seq [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    EXPECT_FALSE(t.is_anchor_or_ref(stream_id));
    EXPECT_FALSE(t.is_anchor_or_ref(doc_id));
    EXPECT_TRUE(t.is_anchor_or_ref(map_id));
    EXPECT_TRUE(t.is_anchor_or_ref(keyval_id));
    EXPECT_TRUE(t.is_anchor_or_ref(seq_id));
    EXPECT_TRUE(t.is_anchor_or_ref(val_id));
    EXPECT_FALSE(stream.is_anchor_or_ref());
    EXPECT_FALSE(doc.is_anchor_or_ref());
    EXPECT_TRUE(map.is_anchor_or_ref());
    EXPECT_TRUE(keyval.is_anchor_or_ref());
    EXPECT_TRUE(seq.is_anchor_or_ref());
    EXPECT_TRUE(val.is_anchor_or_ref());
    EXPECT_EQ(t.is_anchor_or_ref(stream_id), t._p(stream_id)->m_type.is_anchor_or_ref());
    EXPECT_EQ(t.is_anchor_or_ref(doc_id), t._p(doc_id)->m_type.is_anchor_or_ref());
    EXPECT_EQ(t.is_anchor_or_ref(map_id), t._p(map_id)->m_type.is_anchor_or_ref());
    EXPECT_EQ(t.is_anchor_or_ref(keyval_id), t._p(keyval_id)->m_type.is_anchor_or_ref());
    EXPECT_EQ(t.is_anchor_or_ref(seq_id), t._p(seq_id)->m_type.is_anchor_or_ref());
    EXPECT_EQ(t.is_anchor_or_ref(val_id), t._p(val_id)->m_type.is_anchor_or_ref());
    EXPECT_EQ(stream.is_anchor_or_ref(), stream.get()->m_type.is_anchor_or_ref());
    EXPECT_EQ(doc.is_anchor_or_ref(), doc.get()->m_type.is_anchor_or_ref());
    EXPECT_EQ(map.is_anchor_or_ref(), map.get()->m_type.is_anchor_or_ref());
    EXPECT_EQ(keyval.is_anchor_or_ref(), keyval.get()->m_type.is_anchor_or_ref());
    EXPECT_EQ(seq.is_anchor_or_ref(), seq.get()->m_type.is_anchor_or_ref());
    EXPECT_EQ(val.is_anchor_or_ref(), val.get()->m_type.is_anchor_or_ref());
}

TEST(NodeType, is_key_quoted)
{
    EXPECT_FALSE(NodeType().is_key_quoted());
    EXPECT_FALSE(NodeType(KEYQUO).is_key_quoted());
    EXPECT_TRUE(NodeType(KEY|KEYQUO).is_key_quoted());
}

TEST(Tree, is_key_quoted)
{
    Tree t = parse_in_arena(R"(---
"quoted": foo
notquoted: bar
...)");
    const size_t map_id = t.first_child(t.root_id());
    const size_t quoted_id = t.first_child(map_id);
    const size_t notquoted_id = t.last_child(map_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef quoted = t.ref(quoted_id);
    const NodeRef notquoted = t.ref(notquoted_id);
    EXPECT_FALSE(t.is_key_quoted(map_id));
    EXPECT_TRUE(t.is_key_quoted(quoted_id));
    EXPECT_FALSE(t.is_key_quoted(notquoted_id));
    EXPECT_FALSE(map.is_key_quoted());
    EXPECT_TRUE(quoted.is_key_quoted());
    EXPECT_FALSE(notquoted.is_key_quoted());
    EXPECT_EQ(t.is_key_quoted(map_id), t._p(map_id)->m_type.is_key_quoted());
    EXPECT_EQ(t.is_key_quoted(quoted_id), t._p(quoted_id)->m_type.is_key_quoted());
    EXPECT_EQ(t.is_key_quoted(notquoted_id), t._p(notquoted_id)->m_type.is_key_quoted());
    EXPECT_EQ(map.is_key_quoted(), map.get()->m_type.is_key_quoted());
    EXPECT_EQ(quoted.is_key_quoted(), quoted.get()->m_type.is_key_quoted());
    EXPECT_EQ(notquoted.is_key_quoted(), notquoted.get()->m_type.is_key_quoted());
}

TEST(NodeType, is_val_quoted)
{
    EXPECT_FALSE(NodeType().is_val_quoted());
    EXPECT_FALSE(NodeType(VALQUO).is_val_quoted());
    EXPECT_TRUE(NodeType(VAL|VALQUO).is_val_quoted());
}

TEST(Tree, is_val_quoted)
{
    Tree t = parse_in_arena(R"(---
"quoted": "foo"
notquoted: bar
...)");
    const size_t map_id = t.first_child(t.root_id());
    const size_t quoted_id = t.first_child(map_id);
    const size_t notquoted_id = t.last_child(map_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef quoted = t.ref(quoted_id);
    const NodeRef notquoted = t.ref(notquoted_id);
    EXPECT_FALSE(t.is_val_quoted(map_id));
    EXPECT_TRUE(t.is_val_quoted(quoted_id));
    EXPECT_FALSE(t.is_val_quoted(notquoted_id));
    EXPECT_FALSE(map.is_val_quoted());
    EXPECT_TRUE(quoted.is_val_quoted());
    EXPECT_FALSE(notquoted.is_val_quoted());
    EXPECT_EQ(t.is_val_quoted(map_id), t._p(map_id)->m_type.is_val_quoted());
    EXPECT_EQ(t.is_val_quoted(quoted_id), t._p(quoted_id)->m_type.is_val_quoted());
    EXPECT_EQ(t.is_val_quoted(notquoted_id), t._p(notquoted_id)->m_type.is_val_quoted());
    EXPECT_EQ(map.is_val_quoted(), map.get()->m_type.is_val_quoted());
    EXPECT_EQ(quoted.is_val_quoted(), quoted.get()->m_type.is_val_quoted());
    EXPECT_EQ(notquoted.is_val_quoted(), notquoted.get()->m_type.is_val_quoted());
}

TEST(NodeType, is_quoted)
{
    EXPECT_FALSE(NodeType().is_quoted());
    EXPECT_FALSE(NodeType(KEYQUO).is_quoted());
    EXPECT_FALSE(NodeType(VALQUO).is_quoted());
    EXPECT_FALSE(NodeType(KEYQUO|VALQUO).is_quoted());
    EXPECT_TRUE(NodeType(KEY|KEYQUO).is_quoted());
    EXPECT_TRUE(NodeType(VAL|VALQUO).is_quoted());
    EXPECT_FALSE(NodeType(KEY|VALQUO).is_quoted());
    EXPECT_FALSE(NodeType(VAL|KEYQUO).is_quoted());
}

TEST(Tree, is_quoted)
{
    Tree t = parse_in_arena(R"(---
"quoted1": foo
quoted2: "foo"
"quoted3": "foo"
'quoted4': foo
quoted5: 'foo'
'quoted6': 'foo'
notquoted: bar
...)");
    const size_t map_id = t.first_child(t.root_id());
    const size_t quoted1_id = t.find_child(map_id, "quoted1");
    const size_t quoted2_id = t.find_child(map_id, "quoted2");
    const size_t quoted3_id = t.find_child(map_id, "quoted3");
    const size_t quoted4_id = t.find_child(map_id, "quoted4");
    const size_t quoted5_id = t.find_child(map_id, "quoted5");
    const size_t quoted6_id = t.find_child(map_id, "quoted6");
    const size_t notquoted_id = t.last_child(map_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef quoted1 = t.ref(quoted1_id);
    const NodeRef quoted2 = t.ref(quoted2_id);
    const NodeRef quoted3 = t.ref(quoted3_id);
    const NodeRef quoted4 = t.ref(quoted4_id);
    const NodeRef quoted5 = t.ref(quoted5_id);
    const NodeRef quoted6 = t.ref(quoted6_id);
    const NodeRef notquoted = t.ref(notquoted_id);
    EXPECT_FALSE(t.is_quoted(map_id));
    EXPECT_TRUE(t.is_quoted(quoted1_id));
    EXPECT_TRUE(t.is_quoted(quoted2_id));
    EXPECT_TRUE(t.is_quoted(quoted3_id));
    EXPECT_TRUE(t.is_quoted(quoted4_id));
    EXPECT_TRUE(t.is_quoted(quoted5_id));
    EXPECT_TRUE(t.is_quoted(quoted6_id));
    EXPECT_FALSE(t.is_quoted(notquoted_id));
    EXPECT_FALSE(map.is_quoted());
    EXPECT_TRUE(quoted1.is_quoted());
    EXPECT_TRUE(quoted2.is_quoted());
    EXPECT_TRUE(quoted3.is_quoted());
    EXPECT_TRUE(quoted4.is_quoted());
    EXPECT_TRUE(quoted5.is_quoted());
    EXPECT_TRUE(quoted6.is_quoted());
    EXPECT_FALSE(notquoted.is_quoted());
    EXPECT_EQ(t.is_quoted(map_id), t._p(map_id)->m_type.is_quoted());
    EXPECT_EQ(t.is_quoted(quoted1_id), t._p(quoted1_id)->m_type.is_quoted());
    EXPECT_EQ(t.is_quoted(quoted2_id), t._p(quoted2_id)->m_type.is_quoted());
    EXPECT_EQ(t.is_quoted(quoted3_id), t._p(quoted3_id)->m_type.is_quoted());
    EXPECT_EQ(t.is_quoted(quoted4_id), t._p(quoted4_id)->m_type.is_quoted());
    EXPECT_EQ(t.is_quoted(quoted5_id), t._p(quoted5_id)->m_type.is_quoted());
    EXPECT_EQ(t.is_quoted(quoted6_id), t._p(quoted6_id)->m_type.is_quoted());
    EXPECT_EQ(t.is_quoted(notquoted_id), t._p(notquoted_id)->m_type.is_quoted());
    EXPECT_EQ(map.is_quoted(), map.get()->m_type.is_quoted());
    EXPECT_EQ(quoted1.is_quoted(), quoted1.get()->m_type.is_quoted());
    EXPECT_EQ(quoted2.is_quoted(), quoted2.get()->m_type.is_quoted());
    EXPECT_EQ(quoted3.is_quoted(), quoted3.get()->m_type.is_quoted());
    EXPECT_EQ(quoted4.is_quoted(), quoted4.get()->m_type.is_quoted());
    EXPECT_EQ(quoted5.is_quoted(), quoted5.get()->m_type.is_quoted());
    EXPECT_EQ(quoted6.is_quoted(), quoted6.get()->m_type.is_quoted());
    EXPECT_EQ(notquoted.is_quoted(), notquoted.get()->m_type.is_quoted());
}


TEST(Tree, parent_is_seq)
{
    Tree t = parse_in_arena(R"(---
map: {foo: *keyvalref, notag: none}
seq: &seq [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    //EXPECT_FALSE(t.parent_is_seq(stream_id));
    EXPECT_TRUE(t.parent_is_seq(doc_id));
    EXPECT_FALSE(t.parent_is_seq(map_id));
    EXPECT_FALSE(t.parent_is_seq(keyval_id));
    EXPECT_FALSE(t.parent_is_seq(seq_id));
    EXPECT_TRUE(t.parent_is_seq(val_id));
    //EXPECT_FALSE(stream.parent_is_seq());
    EXPECT_TRUE(doc.parent_is_seq());
    EXPECT_FALSE(map.parent_is_seq());
    EXPECT_FALSE(keyval.parent_is_seq());
    EXPECT_FALSE(seq.parent_is_seq());
    EXPECT_TRUE(val.parent_is_seq());
    //EXPECT_EQ(t.parent_is_seq(stream_id), stream.parent_is_seq());
    EXPECT_EQ(t.parent_is_seq(doc_id), doc.parent_is_seq());
    EXPECT_EQ(t.parent_is_seq(map_id), map.parent_is_seq());
    EXPECT_EQ(t.parent_is_seq(keyval_id), keyval.parent_is_seq());
    EXPECT_EQ(t.parent_is_seq(seq_id), seq.parent_is_seq());
    EXPECT_EQ(t.parent_is_seq(val_id), val.parent_is_seq());
}

TEST(Tree, parent_is_map)
{
    Tree t = parse_in_arena(R"(---
map: {foo: *keyvalref, notag: none}
seq: &seq [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    //EXPECT_FALSE(t.parent_is_map(stream_id));
    EXPECT_FALSE(t.parent_is_map(doc_id));
    EXPECT_TRUE(t.parent_is_map(map_id));
    EXPECT_TRUE(t.parent_is_map(keyval_id));
    EXPECT_TRUE(t.parent_is_map(seq_id));
    EXPECT_FALSE(t.parent_is_map(val_id));
    //EXPECT_FALSE(stream.parent_is_map());
    EXPECT_FALSE(doc.parent_is_map());
    EXPECT_TRUE(map.parent_is_map());
    EXPECT_TRUE(keyval.parent_is_map());
    EXPECT_TRUE(seq.parent_is_map());
    EXPECT_FALSE(val.parent_is_map());
    //EXPECT_EQ(t.parent_is_map(stream_id), stream.parent_is_map());
    EXPECT_EQ(t.parent_is_map(doc_id), doc.parent_is_map());
    EXPECT_EQ(t.parent_is_map(map_id), map.parent_is_map());
    EXPECT_EQ(t.parent_is_map(keyval_id), keyval.parent_is_map());
    EXPECT_EQ(t.parent_is_map(seq_id), seq.parent_is_map());
    EXPECT_EQ(t.parent_is_map(val_id), val.parent_is_map());
}

TEST(Tree, has_parent)
{
    Tree t = parse_in_arena(R"(---
map: {foo: *keyvalref, notag: none}
seq: &seq [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    EXPECT_FALSE(t.has_parent(stream_id));
    EXPECT_TRUE(t.has_parent(doc_id));
    EXPECT_TRUE(t.has_parent(map_id));
    EXPECT_TRUE(t.has_parent(keyval_id));
    EXPECT_TRUE(t.has_parent(seq_id));
    EXPECT_TRUE(t.has_parent(val_id));
    EXPECT_FALSE(stream.has_parent());
    EXPECT_TRUE(doc.has_parent());
    EXPECT_TRUE(map.has_parent());
    EXPECT_TRUE(keyval.has_parent());
    EXPECT_TRUE(seq.has_parent());
    EXPECT_TRUE(val.has_parent());
    EXPECT_EQ(t.has_parent(stream_id), stream.has_parent());
    EXPECT_EQ(t.has_parent(doc_id), doc.has_parent());
    EXPECT_EQ(t.has_parent(map_id), map.has_parent());
    EXPECT_EQ(t.has_parent(keyval_id), keyval.has_parent());
    EXPECT_EQ(t.has_parent(seq_id), seq.has_parent());
    EXPECT_EQ(t.has_parent(val_id), val.has_parent());
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TEST(Tree, num_children)
{
    Tree t = parse_in_arena(R"(---
map: {foo: *keyvalref, notag: none}
seq: &seq [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    EXPECT_EQ(t.num_children(stream_id), 1u);
    EXPECT_EQ(t.num_children(doc_id), 2u);
    EXPECT_EQ(t.num_children(map_id), 2u);
    EXPECT_EQ(t.num_children(keyval_id), 0u);
    EXPECT_EQ(t.num_children(seq_id), 2u);
    EXPECT_EQ(t.num_children(val_id), 0u);
    EXPECT_EQ(stream.num_children(), t.num_children(stream_id));
    EXPECT_EQ(doc.num_children(), t.num_children(doc_id));
    EXPECT_EQ(map.num_children(), t.num_children(map_id));
    EXPECT_EQ(keyval.num_children(), t.num_children(keyval_id));
    EXPECT_EQ(seq.num_children(), t.num_children(seq_id));
    EXPECT_EQ(val.num_children(), t.num_children(val_id));
}

TEST(Tree, child)
{
    Tree t = parse_in_arena(R"(---
map: {foo: *keyvalref, notag: none}
seq: &seq [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const size_t val_id = t.first_child(seq_id);
    const NodeRef stream = t.ref(stream_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    const NodeRef keyval = t.ref(keyval_id);
    const NodeRef seq = t.ref(seq_id);
    const NodeRef val = t.ref(val_id);
    EXPECT_EQ(t.child(stream_id, 0), doc_id);
    EXPECT_EQ(t.child(doc_id, 0), map_id);
    EXPECT_EQ(t.child(map_id, 0), keyval_id);
    EXPECT_EQ(t.child(keyval_id, 0), (size_t)NONE);
    EXPECT_EQ(t.child(seq_id, 0), val_id);
    EXPECT_EQ(t.child(val_id, 0), (size_t)NONE);
    EXPECT_EQ(stream.child(0).id(), t.child(stream_id, 0));
    EXPECT_EQ(doc.child(0).id(), t.child(doc_id, 0));
    EXPECT_EQ(map.child(0).id(), t.child(map_id, 0));
    EXPECT_EQ(keyval.child(0).id(), t.child(keyval_id, 0));
    EXPECT_EQ(seq.child(0).id(), t.child(seq_id, 0));
    EXPECT_EQ(val.child(0).id(), t.child(val_id, 0));
}

TEST(Tree, find_child_by_name)
{
    Tree t = parse_in_arena(R"(---
map: {foo: *keyvalref, notag: none}
seq: &seq [*valref, bar]
...)");
    const size_t stream_id = t.root_id();
    const size_t doc_id = t.first_child(stream_id);
    const size_t map_id = t.first_child(doc_id);
    const size_t keyval_id = t.first_child(map_id);
    const size_t seq_id = t.last_child(doc_id);
    const NodeRef doc = t.ref(doc_id);
    const NodeRef map = t.ref(map_id);
    EXPECT_EQ(t.find_child(doc_id, "map"), map_id);
    EXPECT_EQ(t.find_child(doc_id, "seq"), seq_id);
    EXPECT_EQ(t.find_child(doc_id, "..."), (size_t)NONE);
    EXPECT_EQ(t.find_child(map_id, "foo"), keyval_id);
    EXPECT_EQ(t.find_child(map_id, "bar"), (size_t)NONE);
    EXPECT_EQ(doc.find_child("map").id(), t.find_child(doc_id, "map"));
    EXPECT_EQ(doc.find_child("seq").id(), t.find_child(doc_id, "seq"));
    EXPECT_EQ(doc.find_child("...").id(), t.find_child(doc_id, "..."));
    EXPECT_EQ(map.find_child("foo").id(), t.find_child(map_id, "foo"));
    EXPECT_EQ(map.find_child("bar").id(), t.find_child(map_id, "bar"));
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TEST(NodeInit, ctor__empty)
{
    NodeInit n;
    EXPECT_EQ((type_bits)n.type, (type_bits)NOTYPE);
    EXPECT_EQ(n.key.scalar, "");
    EXPECT_EQ(n.key.tag, "");
    EXPECT_EQ(n.val.scalar, "");
    EXPECT_EQ(n.val.tag, "");
}

TEST(NodeInit, ctor__type_only)
{
    for(auto k : {KEY, KEYVAL, MAP, KEYMAP, SEQ, KEYSEQ})
    {
        SCOPED_TRACE(NodeType::type_str(k));
        NodeInit n(k);
        EXPECT_EQ((type_bits)n.type, (type_bits)k);
        EXPECT_EQ(n.key.scalar, "");
        EXPECT_EQ(n.key.tag, "");
        EXPECT_EQ(n.val.scalar, "");
        EXPECT_EQ(n.val.tag, "");
    }
}

TEST(NodeInit, ctor__val_only)
{
    {
        const char sarr[] = "foo";
        const char *sptr = "foo"; size_t sptrlen = 3;
        csubstr ssp = "foo";

        {
            SCOPED_TRACE("here 0");
            {
                NodeInit s(sarr);
                node_scalar_test_foo(s.val);
                node_scalar_test_empty(s.key);
                s.clear();
            }
            {
                NodeInit s{to_csubstr(sptr)};
                node_scalar_test_foo(s.val);
                node_scalar_test_empty(s.key);
                s.clear();
            }
            {
                NodeInit s{sarr};
                node_scalar_test_foo(s.val);
                node_scalar_test_empty(s.key);
                s.clear();
            }
        }

        {
            SCOPED_TRACE("here 1");
            {
                NodeInit s(sarr);
                node_scalar_test_foo(s.val);
                node_scalar_test_empty(s.key);
                s.clear();
            }
            {
                NodeInit s(to_csubstr(sptr));
                node_scalar_test_foo(s.val);
                node_scalar_test_empty(s.key);
                s.clear();
            }
            {
                NodeInit s(sarr);
                node_scalar_test_foo(s.val);
                node_scalar_test_empty(s.key);
                s.clear();
            }
        }

        {
            SCOPED_TRACE("here 2");
            NodeInit s;
            s = {sarr};
            node_scalar_test_foo(s.val);
            node_scalar_test_empty(s.key);
            s.clear();
            s = {to_csubstr(sptr)};
            node_scalar_test_foo(s.val);
            node_scalar_test_empty(s.key);
            s.clear();
            //s = {sptr, sptrlen}; // fails to compile
            //node_scalar_test_foo(s.val);
            //node_scalar_test_empty(s.key);
            //s.clear();
            s = {ssp};
            node_scalar_test_foo(s.val);
            node_scalar_test_empty(s.key);
            s.clear();
        }

        for(auto s : {
            NodeInit(sarr),
            NodeInit(to_csubstr(sptr)),
            NodeInit(csubstr{sptr, sptrlen}),
            NodeInit(ssp)})
        {
            SCOPED_TRACE("here LOOP");
            node_scalar_test_foo(s.val);
            node_scalar_test_empty(s.key);
        }
    }

    {
        const char sarr[] = "foo3";
        const char *sptr = "foo3"; size_t sptrlen = 4;
        csubstr ssp = "foo3";

        {
            SCOPED_TRACE("here 0");
            NodeInit s = {sarr};
            node_scalar_test_foo3(s.val);
            node_scalar_test_empty(s.key);
        }
        {   // FAILS
            SCOPED_TRACE("here 1");
            //NodeInit s = sarr;
            //node_scalar_test_foo3(s.val);
            //node_scalar_test_empty(s.key);
        }
        {
            SCOPED_TRACE("here 2");
            NodeInit s{sarr};
            node_scalar_test_foo3(s.val);
            node_scalar_test_empty(s.key);
        }
        {
            SCOPED_TRACE("here 3");
            NodeInit s(sarr);
            node_scalar_test_foo3(s.val);
            node_scalar_test_empty(s.key);
        }

        for(auto s : {
            NodeInit(sarr),
            NodeInit(to_csubstr(sptr)),
            NodeInit(csubstr{sptr, sptrlen}),
            NodeInit(ssp)})
        {
            SCOPED_TRACE("here LOOP");
            node_scalar_test_foo3(s.val);
            node_scalar_test_empty(s.key);
        }
    }
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
TEST(change_type, from_val)
{
    Tree t = parse_in_arena("[val0, val1, val2]");
    t[0].change_type(VAL);
    t[1].change_type(MAP);
    t[2].change_type(SEQ);
    Tree expected = parse_in_arena("[val0, {}, []]");
    EXPECT_EQ(emitrs<std::string>(t), emitrs<std::string>(expected));
}
TEST(change_type, from_keyval)
{
    Tree t = parse_in_arena("{keyval0: val0, keyval1: val1, keyval2: val2}");
    t[0].change_type(VAL);
    t[1].change_type(MAP);
    t[2].change_type(SEQ);
    Tree expected = parse_in_arena("{keyval0: val0, keyval1: {}, keyval2: []}");
    EXPECT_EQ(emitrs<std::string>(t), emitrs<std::string>(expected));
}

TEST(change_type, from_map)
{
    Tree t = parse_in_arena("[{map0: val0}, {map1: {map1key0: a, map1key1: b}}, {map2: [map2val0, map2val1]}]");
    t[0].change_type(VAL);
    t[1].change_type(MAP);
    t[2].change_type(SEQ);
    Tree expected = parse_in_arena("[~, {map1: {map1key0: a, map1key1: b}}, []]");
    EXPECT_EQ(emitrs<std::string>(t), emitrs<std::string>(expected));
}
TEST(change_type, from_keymap)
{
    Tree t = parse_in_arena("{map0: {map0: val0}, map1: {map1: {map1key0: a, map1key1: b}}, map2: {map2: [map2val0, map2val1]}}");
    t[0].change_type(VAL);
    t[1].change_type(MAP);
    t[2].change_type(SEQ);
    Tree expected = parse_in_arena("{map0: ~, map1: {map1: {map1key0: a, map1key1: b}}, map2: []}");
    EXPECT_EQ(emitrs<std::string>(t), emitrs<std::string>(expected));
}

TEST(change_type, from_seq)
{
    Tree t = parse_in_arena("[[seq00, seq01], [seq10, seq11], [seq20, seq21]]");
    t[0].change_type(VAL);
    t[1].change_type(MAP);
    t[2].change_type(SEQ);
    Tree expected = parse_in_arena("[~, {}, [seq20, seq21]]");
    EXPECT_EQ(emitrs<std::string>(t), emitrs<std::string>(expected));
}
TEST(change_type, from_keyseq)
{
    Tree t = parse_in_arena("{map0: [seq00, seq01], map1: [seq10, seq11], map2: [seq20, seq21]}");
    t[0].change_type(VAL);
    t[1].change_type(MAP);
    t[2].change_type(SEQ);
    Tree expected = parse_in_arena("{map0: ~, map1: {}, map2: [seq20, seq21]}");
    EXPECT_EQ(emitrs<std::string>(t), emitrs<std::string>(expected));
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

TEST(NodeRef, 0_general)
{
    Tree t;

    NodeRef root(&t);

    //using S = csubstr;
    //using V = NodeScalar;
    using N = NodeInit;

    root = N{MAP};
    root.append_child({"a", "0"});
    root.append_child({MAP, "b"});
    root["b"].append_child({SEQ, "seq"});
    root["b"]["seq"].append_child({"0"});
    root["b"]["seq"].append_child({"1"});
    root["b"]["seq"].append_child({"2"});
    root["b"]["seq"].append_child({NodeScalar{"!!str", "3"}});
    auto ch4 = root["b"]["seq"][3].append_sibling({"4"});
    EXPECT_EQ(ch4.id(), root["b"]["seq"][4].id());
    EXPECT_EQ(ch4.get(), root["b"]["seq"][4].get());
    EXPECT_EQ((type_bits)root["b"]["seq"][4].type(), (type_bits)VAL);
    EXPECT_EQ(root["b"]["seq"][4].val(), "4");
    root["b"]["seq"].append_sibling({NodeScalar{"!!str", "aaa"}, NodeScalar{"!!int", "0"}});
    EXPECT_EQ((type_bits)root["b"]["seq"][4].type(), (type_bits)VAL);
    EXPECT_EQ(root["b"]["seq"][4].val(), "4");

    root["b"]["key"] = "val";
    auto seq = root["b"]["seq"];
    auto seq2 = root["b"]["seq2"];
    EXPECT_TRUE(seq2.is_seed());
    root["b"]["seq2"] = N(SEQ);
    seq2 = root["b"]["seq2"];
    EXPECT_FALSE(seq2.is_seed());
    EXPECT_TRUE(seq2.is_seq());
    EXPECT_EQ(seq2.num_children(), 0);
    EXPECT_EQ(root["b"]["seq2"].get(), seq2.get());
    auto seq20 = seq2[0];
    EXPECT_TRUE(seq20.is_seed());
    EXPECT_TRUE(seq2[0].is_seed());
    EXPECT_EQ(seq2.num_children(), 0);
    EXPECT_TRUE(seq2[0].is_seed());
    EXPECT_TRUE(seq20.is_seed());
    EXPECT_NE(seq.get(), seq2.get());
    seq20 = root["b"]["seq2"][0];
    EXPECT_TRUE(seq20.is_seed());
    root["b"]["seq2"][0] = "00";
    seq20 = root["b"]["seq2"][0];
    EXPECT_FALSE(seq20.is_seed());
    NodeRef before = root["b"]["key"];
    EXPECT_EQ(before.key(), "key");
    EXPECT_EQ(before.val(), "val");
    root["b"]["seq2"][1] = "01";
    NodeRef after = root["b"]["key"];
    EXPECT_EQ(before.key(), "key");
    EXPECT_EQ(before.val(), "val");
    EXPECT_EQ(after.key(), "key");
    EXPECT_EQ(after.val(), "val");
    root["b"]["seq2"][2] = "02";
    root["b"]["seq2"][3] = "03";
    int iv = 0;
    root["b"]["seq2"][4] << 55; root["b"]["seq2"][4] >> iv;
    size_t zv = 0;
    root["b"]["seq2"][5] << size_t(55); root["b"]["seq2"][5] >> zv;
    float fv = 0;
    root["b"]["seq2"][6] << 2.0f; root["b"]["seq2"][6] >> fv;
    float dv = 0;
    root["b"]["seq2"][7] << 2.0; root["b"]["seq2"][7] >> dv;

    EXPECT_EQ(root["b"]["key"].key(), "key");
    EXPECT_EQ(root["b"]["key"].val(), "val");


    emit(t);

    EXPECT_EQ((type_bits)root.type(), (type_bits)MAP);
    EXPECT_EQ((type_bits)root["a"].type(), (type_bits)KEYVAL);
    EXPECT_EQ(root["a"].key(), "a");
    EXPECT_EQ(root["a"].val(), "0");

    EXPECT_EQ((type_bits)root["b"].type(), (type_bits)KEYMAP);

    EXPECT_EQ((type_bits)root["b"]["seq"].type(), (type_bits)KEYSEQ);
    EXPECT_EQ(           root["b"]["seq"].key(), "seq");
    EXPECT_EQ((type_bits)root["b"]["seq"][0].type(), (type_bits)VAL);
    EXPECT_EQ(           root["b"]["seq"][0].val(), "0");
    EXPECT_EQ((type_bits)root["b"]["seq"][1].type(), (type_bits)VAL);
    EXPECT_EQ(           root["b"]["seq"][1].val(), "1");
    EXPECT_EQ((type_bits)root["b"]["seq"][2].type(), (type_bits)VAL);
    EXPECT_EQ(           root["b"]["seq"][2].val(), "2");
    EXPECT_EQ((type_bits)root["b"]["seq"][3].type(), (type_bits)VAL);
    EXPECT_EQ(           root["b"]["seq"][3].val(), "3");
    EXPECT_EQ(           root["b"]["seq"][3].val_tag(), "!!str");
    EXPECT_EQ((type_bits)root["b"]["seq"][4].type(), (type_bits)VAL);
    EXPECT_EQ(           root["b"]["seq"][4].val(), "4");

    int tv;
    EXPECT_EQ(root["b"]["key"].key(), "key");
    EXPECT_EQ(root["b"]["key"].val(), "val");
    EXPECT_EQ(root["b"]["seq2"][0].val(), "00"); root["b"]["seq2"][0] >> tv; EXPECT_EQ(tv, 0);
    EXPECT_EQ(root["b"]["seq2"][1].val(), "01"); root["b"]["seq2"][1] >> tv; EXPECT_EQ(tv, 1);
    EXPECT_EQ(root["b"]["seq2"][2].val(), "02"); root["b"]["seq2"][2] >> tv; EXPECT_EQ(tv, 2);
    EXPECT_EQ(root["b"]["seq2"][3].val(), "03"); root["b"]["seq2"][3] >> tv; EXPECT_EQ(tv, 3);
    EXPECT_EQ(root["b"]["seq2"][4].val(), "55"); EXPECT_EQ(iv, 55);
    EXPECT_EQ(root["b"]["seq2"][5].val(), "55"); EXPECT_EQ(zv, size_t(55));
    EXPECT_EQ(root["b"]["seq2"][6].val(), "2"); EXPECT_EQ(fv, 2.f);
    EXPECT_EQ(root["b"]["seq2"][6].val(), "2"); EXPECT_EQ(dv, 2.);

    root["b"]["seq"][2].set_val_serialized(22);

    emit(t);

    EXPECT_EQ((type_bits)root["b"]["aaa"].type(), (type_bits)KEYVAL);
    EXPECT_EQ(root["b"]["aaa"].key_tag(), "!!str");
    EXPECT_EQ(root["b"]["aaa"].key(), "aaa");
    EXPECT_EQ(root["b"]["aaa"].val_tag(), "!!int");
    EXPECT_EQ(root["b"]["aaa"].val(), "0");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void noderef_check_tree(NodeRef const& root)
{
    test_invariants(*root.tree());

    EXPECT_EQ(root.tree()->size(), 7u);
    EXPECT_EQ(root.num_children(), 6u);
    EXPECT_EQ(root.is_container(), true);
    EXPECT_EQ(root.is_seq(), true);

    EXPECT_EQ((type_bits)root[0].type(), (type_bits)VAL);
    EXPECT_EQ(           root[0].val(), "0");
    EXPECT_EQ((type_bits)root[1].type(), (type_bits)VAL);
    EXPECT_EQ(           root[1].val(), "1");
    EXPECT_EQ((type_bits)root[2].type(), (type_bits)VAL);
    EXPECT_EQ(           root[2].val(), "2");
    EXPECT_EQ((type_bits)root[3].type(), (type_bits)VAL);
    EXPECT_EQ(           root[3].val(), "3");
    EXPECT_EQ((type_bits)root[4].type(), (type_bits)VAL);
    EXPECT_EQ(           root[4].val(), "4");
    EXPECT_EQ((type_bits)root[5].type(), (type_bits)VAL);
    EXPECT_EQ(           root[5].val(), "5");
}

TEST(NodeRef, 1_append_child)
{
    Tree t;

    NodeRef root(&t);

    root |= SEQ;
    root.append_child({"0"});
    root.append_child({"1"});
    root.append_child({"2"});
    root.append_child({"3"});
    root.append_child({"4"});
    root.append_child({"5"});

    noderef_check_tree(root);
}

TEST(NodeRef, 2_prepend_child)
{
    Tree t;

    NodeRef root(&t);

    root |= SEQ;
    root.prepend_child({"5"});
    root.prepend_child({"4"});
    root.prepend_child({"3"});
    root.prepend_child({"2"});
    root.prepend_child({"1"});
    root.prepend_child({"0"});

    noderef_check_tree(root);
}

TEST(NodeRef, 3_insert_child)
{
    Tree t;

    NodeRef root(&t);
    NodeRef none(&t, NONE);

    root |= SEQ;
    root.insert_child({"3"}, none);
    root.insert_child({"4"}, root[0]);
    root.insert_child({"0"}, none);
    root.insert_child({"5"}, root[2]);
    root.insert_child({"1"}, root[0]);
    root.insert_child({"2"}, root[1]);

    noderef_check_tree(root);
}

TEST(NodeRef, 4_remove_child)
{
    Tree t;

    NodeRef root(&t);
    NodeRef none(&t, NONE);

    root |= SEQ;
    root.insert_child({"3"}, none);
    root.insert_child({"4"}, root[0]);
    root.insert_child({"0"}, none);
    root.insert_child({"5"}, root[2]);
    root.insert_child({"1"}, root[0]);
    root.insert_child({"2"}, root[1]);

    std::vector<int> vec({10, 20, 30, 40, 50, 60, 70, 80, 90});
    root.insert_child(root[0]) << vec; // 1
    root.insert_child(root[2]) << vec; // 3
    root.insert_child(root[4]) << vec; // 5
    root.insert_child(root[6]) << vec; // 7
    root.insert_child(root[8]) << vec; // 9
    root.append_child() << vec;        // 10

    root.remove_child(11);
    root.remove_child(9);
    root.remove_child(7);
    root.remove_child(5);
    root.remove_child(3);
    root.remove_child(1);

    noderef_check_tree(root);

    std::vector<std::vector<int>> vec2({{100, 200}, {300, 400}, {500, 600}, {700, 800, 900}});
    root.prepend_child() << vec2; // 0
    root.insert_child(root[1]) << vec2; // 2
    root.insert_child(root[3]) << vec2; // 4
    root.insert_child(root[5]) << vec2; // 6
    root.insert_child(root[7]) << vec2; // 8
    root.insert_child(root[9]) << vec2; // 10
    root.append_child() << vec2;        // 12

    root.remove_child(12);
    root.remove_child(10);
    root.remove_child(8);
    root.remove_child(6);
    root.remove_child(4);
    root.remove_child(2);
    root.remove_child(0);

    noderef_check_tree(root);
}

TEST(NodeRef, 5_move_in_same_parent)
{
    Tree t;
    NodeRef r = t;

    std::vector<std::vector<int>> vec2({{100, 200}, {300, 400}, {500, 600}, {700, 800, 900}});
    std::map<std::string, int> map2({{"foo", 100}, {"bar", 200}, {"baz", 300}});

    r |= SEQ;
    r.append_child() << vec2;
    r.append_child() << map2;
    r.append_child() << "elm2";
    r.append_child() << "elm3";

    auto s = r[0];
    auto m = r[1];
    EXPECT_TRUE(s.is_seq());
    EXPECT_TRUE(m.is_map());
    EXPECT_EQ(s.num_children(), vec2.size());
    EXPECT_EQ(m.num_children(), map2.size());
    //printf("fonix"); print_tree(t); emit(r);
    r[0].move(r[1]);
    //printf("fonix"); print_tree(t); emit(r);
    EXPECT_EQ(r[0].get(), m.get());
    EXPECT_EQ(r[0].num_children(), map2.size());
    EXPECT_EQ(r[1].get(), s.get());
    EXPECT_EQ(r[1].num_children(), vec2.size());
}

TEST(NodeRef, 6_move_to_other_parent)
{
    Tree t;
    NodeRef r = t;

    std::vector<std::vector<int>> vec2({{100, 200}, {300, 400}, {500, 600}, {700, 800, 900}});
    std::map<std::string, int> map2({{"foo", 100}, {"bar", 200}, {"baz", 300}});

    r |= SEQ;
    r.append_child() << vec2;
    r.append_child() << map2;
    r.append_child() << "elm2";
    r.append_child() << "elm3";

    NodeData *elm2 = r[2].get();
    EXPECT_EQ(r[2].val(), "elm2");
    //printf("fonix"); print_tree(t); emit(r);
    r[2].move(r[0], r[0][0]);
    EXPECT_EQ(r[0][1].get(), elm2);
    EXPECT_EQ(r[0][1].val(), "elm2");
    //printf("fonix"); print_tree(t); emit(r);
}

TEST(NodeRef, 7_duplicate)
{
    Tree t;
    NodeRef r = t;

    std::vector<std::vector<int>> vec2({{100, 200}, {300, 400}, {500, 600}, {700, 800, 900}});
    std::map<std::string, int> map2({{"bar", 200}, {"baz", 300}, {"foo", 100}});

    r |= SEQ;
    r.append_child() << vec2;
    r.append_child() << map2;
    r.append_child() << "elm2";
    r.append_child() << "elm3";

    EXPECT_EQ(r[0][0].num_children(), 2u);
    NodeRef dup = r[1].duplicate(r[0][0], r[0][0][1]);
    EXPECT_EQ(r[0][0].num_children(), 3u);
    EXPECT_EQ(r[0][0][2].num_children(), map2.size());
    EXPECT_NE(dup.get(), r[1].get());
    EXPECT_EQ(dup[0].key(), "bar");
    EXPECT_EQ(dup[0].val(), "200");
    EXPECT_EQ(dup[1].key(), "baz");
    EXPECT_EQ(dup[1].val(), "300");
    EXPECT_EQ(dup[2].key(), "foo");
    EXPECT_EQ(dup[2].val(), "100");
    EXPECT_EQ(dup[0].key().str, r[1][0].key().str);
    EXPECT_EQ(dup[0].val().str, r[1][0].val().str);
    EXPECT_EQ(dup[0].key().len, r[1][0].key().len);
    EXPECT_EQ(dup[0].val().len, r[1][0].val().len);
    EXPECT_EQ(dup[1].key().str, r[1][1].key().str);
    EXPECT_EQ(dup[1].val().str, r[1][1].val().str);
    EXPECT_EQ(dup[1].key().len, r[1][1].key().len);
    EXPECT_EQ(dup[1].val().len, r[1][1].val().len);
}

TEST(NodeRef, intseq)
{
    Tree t = parse_in_arena("iseq: [8, 10]");
    NodeRef n = t["iseq"];
    int a, b;
    n[0] >> a;
    n[1] >> b;
    EXPECT_EQ(a, 8);
    EXPECT_EQ(b, 10);
}


TEST(Tree, lookup_path)
{
    const char yaml[] = R"(
a:
  b: bval
  c:
    d:
      - e
      - d
      - f: fval
        g: gval
        h:
          -
            x: a
            y: b
          -
            z: c
            u:
)";
    Tree t = parse_in_arena(yaml);
    print_tree(t);

    EXPECT_EQ(t.lookup_path("a").target, 1);
    EXPECT_EQ(t.lookup_path("a.b").target, 2);
    EXPECT_EQ(t.lookup_path("a.c").target, 3);
    EXPECT_EQ(t.lookup_path("a.c.d").target, 4);
    EXPECT_EQ(t.lookup_path("a.c.d[0]").target, 5);
    EXPECT_EQ(t.lookup_path("a.c.d[1]").target, 6);
    EXPECT_EQ(t.lookup_path("a.c.d[2]").target, 7);
    EXPECT_EQ(t.lookup_path("a.c.d[2].f").target, 8);
    EXPECT_EQ(t.lookup_path("a.c.d[2].g").target, 9);
    EXPECT_EQ(t.lookup_path("a.c.d[2].h").target, 10);
    EXPECT_EQ(t.lookup_path("a.c.d[2].h[0]").target, 11);
    EXPECT_EQ(t.lookup_path("a.c.d[2].h[0].x").target, 12);
    EXPECT_EQ(t.lookup_path("a.c.d[2].h[0].y").target, 13);
    EXPECT_EQ(t.lookup_path("a.c.d[2].h[1]").target, 14);
    EXPECT_EQ(t.lookup_path("a.c.d[2].h[1].z").target, 15);
    EXPECT_EQ(t.lookup_path("a.c.d[2].h[1].u").target, 16);
    EXPECT_EQ(t.lookup_path("d", 3).target, 4);
    EXPECT_EQ(t.lookup_path("d[0]", 3).target, 5);
    EXPECT_EQ(t.lookup_path("d[1]", 3).target, 6);
    EXPECT_EQ(t.lookup_path("d[2]", 3).target, 7);
    EXPECT_EQ(t.lookup_path("d[2].f", 3).target, 8);
    EXPECT_EQ(t.lookup_path("d[2].g", 3).target, 9);
    EXPECT_EQ(t.lookup_path("d[2].h", 3).target, 10);
    EXPECT_EQ(t.lookup_path("d[2].h[0]", 3).target, 11);
    EXPECT_EQ(t.lookup_path("d[2].h[0].x", 3).target, 12);
    EXPECT_EQ(t.lookup_path("d[2].h[0].y", 3).target, 13);
    EXPECT_EQ(t.lookup_path("d[2].h[1]", 3).target, 14);
    EXPECT_EQ(t.lookup_path("d[2].h[1].z", 3).target, 15);
    EXPECT_EQ(t.lookup_path("d[2].h[1].u", 3).target, 16);

    auto lp = t.lookup_path("x");
    EXPECT_FALSE(lp);
    EXPECT_EQ(lp.target, (size_t)NONE);
    EXPECT_EQ(lp.closest, (size_t)NONE);
    EXPECT_EQ(lp.resolved(), "");
    EXPECT_EQ(lp.unresolved(), "x");
    lp = t.lookup_path("a.x");
    EXPECT_FALSE(lp);
    EXPECT_EQ(lp.target, (size_t)NONE);
    EXPECT_EQ(lp.closest, 1);
    EXPECT_EQ(lp.resolved(), "a");
    EXPECT_EQ(lp.unresolved(), "x");
    lp = t.lookup_path("a.b.x");
    EXPECT_FALSE(lp);
    EXPECT_EQ(lp.target, (size_t)NONE);
    EXPECT_EQ(lp.closest, 2);
    EXPECT_EQ(lp.resolved(), "a.b");
    EXPECT_EQ(lp.unresolved(), "x");
    lp = t.lookup_path("a.c.x");
    EXPECT_FALSE(lp);
    EXPECT_EQ(lp.target, (size_t)NONE);
    EXPECT_EQ(lp.closest, 3);
    EXPECT_EQ(lp.resolved(), "a.c");
    EXPECT_EQ(lp.unresolved(), "x");

    size_t sz = t.size();
    EXPECT_EQ(t.lookup_path("x").target, (size_t)NONE);
    EXPECT_EQ(t.lookup_path_or_modify("x", "x"), sz);
    EXPECT_EQ(t.lookup_path("x").target, sz);
    EXPECT_EQ(t.val(sz), "x");
    EXPECT_EQ(t.lookup_path_or_modify("y", "x"), sz);
    EXPECT_EQ(t.val(sz), "y");
    EXPECT_EQ(t.lookup_path_or_modify("z", "x"), sz);
    EXPECT_EQ(t.val(sz), "z");

    sz = t.size();
    EXPECT_EQ(t.lookup_path("a.x").target, (size_t)NONE);
    EXPECT_EQ(t.lookup_path_or_modify("x", "a.x"), sz);
    EXPECT_EQ(t.lookup_path("a.x").target, sz);
    EXPECT_EQ(t.val(sz), "x");
    EXPECT_EQ(t.lookup_path_or_modify("y", "a.x"), sz);
    EXPECT_EQ(t.val(sz), "y");
    EXPECT_EQ(t.lookup_path_or_modify("z", "a.x"), sz);
    EXPECT_EQ(t.val(sz), "z");

    sz = t.size();
    EXPECT_EQ(t.lookup_path("a.c.x").target, (size_t)NONE);
    EXPECT_EQ(t.lookup_path_or_modify("x", "a.c.x"), sz);
    EXPECT_EQ(t.lookup_path("a.c.x").target, sz);
    EXPECT_EQ(t.val(sz), "x");
    EXPECT_EQ(t.lookup_path_or_modify("y", "a.c.x"), sz);
    EXPECT_EQ(t.val(sz), "y");
    EXPECT_EQ(t.lookup_path_or_modify("z", "a.c.x"), sz);
    EXPECT_EQ(t.val(sz), "z");
}

TEST(Tree, lookup_path_or_modify)
{
    {
        Tree dst = parse_in_arena("{}");
        Tree const src = parse_in_arena("{d: [x, y, z]}");
        dst.lookup_path_or_modify("ok", "a.b.c");
        EXPECT_EQ(dst["a"]["b"]["c"].val(), "ok");
        dst.lookup_path_or_modify(&src, src["d"].id(), "a.b.d");
        EXPECT_EQ(dst["a"]["b"]["d"][0].val(), "x");
        EXPECT_EQ(dst["a"]["b"]["d"][1].val(), "y");
        EXPECT_EQ(dst["a"]["b"]["d"][2].val(), "z");
    }

    {
        Tree t = parse_in_arena("{}");
        csubstr bigpath = "newmap.newseq[0].newmap.newseq[0].first";
        auto result = t.lookup_path(bigpath);
        EXPECT_EQ(result.target, (size_t)NONE);
        EXPECT_EQ(result.closest, (size_t)NONE);
        EXPECT_EQ(result.resolved(), "");
        EXPECT_EQ(result.unresolved(), bigpath);
        size_t sz = t.lookup_path_or_modify("x", bigpath);
        EXPECT_EQ(t.lookup_path(bigpath).target, sz);
        EXPECT_EQ(t.val(sz), "x");
        EXPECT_EQ(t["newmap"]["newseq"].num_children(), 1u);
        EXPECT_EQ(t["newmap"]["newseq"][0].is_map(), true);
        EXPECT_EQ(t["newmap"]["newseq"][0]["newmap"].is_map(), true);
        EXPECT_EQ(t["newmap"]["newseq"][0]["newmap"]["newseq"].is_seq(), true);
        EXPECT_EQ(t["newmap"]["newseq"][0]["newmap"]["newseq"].num_children(), 1u);
        EXPECT_EQ(t["newmap"]["newseq"][0]["newmap"]["newseq"][0].is_map(), true);
        EXPECT_EQ(t["newmap"]["newseq"][0]["newmap"]["newseq"][0]["first"].val(), "x");
        size_t sz2 = t.lookup_path_or_modify("y", bigpath);
        EXPECT_EQ(t["newmap"]["newseq"][0]["newmap"]["newseq"][0]["first"].val(), "y");
        EXPECT_EQ(sz2, sz);
        EXPECT_EQ(t.lookup_path(bigpath).target, sz);
        EXPECT_EQ(t.val(sz2), "y");

        sz2 = t.lookup_path_or_modify("y", "newmap2.newseq2[2].newmap2.newseq2[3].first2");
        EXPECT_EQ(t.lookup_path("newmap2.newseq2[2].newmap2.newseq2[3].first2").target, sz2);
        EXPECT_EQ(t.val(sz2), "y");
        EXPECT_EQ(t["newmap2"]["newseq2"].num_children(), 3u);
        EXPECT_EQ(t["newmap2"]["newseq2"][0].val(), nullptr);
        EXPECT_EQ(t["newmap2"]["newseq2"][1].val(), nullptr);
        EXPECT_EQ(t["newmap2"]["newseq2"][2].is_map(), true);
        EXPECT_EQ(t["newmap2"]["newseq2"][2]["newmap2"].is_map(), true);
        EXPECT_EQ(t["newmap2"]["newseq2"][2]["newmap2"]["newseq2"].is_seq(), true);
        EXPECT_EQ(t["newmap2"]["newseq2"][2]["newmap2"]["newseq2"].num_children(), 4u);
        EXPECT_EQ(t["newmap2"]["newseq2"][2]["newmap2"]["newseq2"][0].val(), nullptr);
        EXPECT_EQ(t["newmap2"]["newseq2"][2]["newmap2"]["newseq2"][1].val(), nullptr);
        EXPECT_EQ(t["newmap2"]["newseq2"][2]["newmap2"]["newseq2"][2].val(), nullptr);
        EXPECT_EQ(t["newmap2"]["newseq2"][2]["newmap2"]["newseq2"][3].is_map(), true);
        EXPECT_EQ(t["newmap2"]["newseq2"][2]["newmap2"]["newseq2"][3]["first2"].val(), "y");
        sz2 = t.lookup_path_or_modify("z", "newmap2.newseq2[2].newmap2.newseq2[3].second2");
        EXPECT_EQ  (t["newmap2"]["newseq2"][2]["newmap2"]["newseq2"][3]["second2"].val(), "z");

        sz = t.lookup_path_or_modify("foo", "newmap.newseq1[1]");
        EXPECT_EQ(t["newmap"].is_map(), true);
        EXPECT_EQ(t["newmap"]["newseq1"].is_seq(), true);
        EXPECT_EQ(t["newmap"]["newseq1"].num_children(), 2u);
        EXPECT_EQ(t["newmap"]["newseq1"][0].val(), nullptr);
        EXPECT_EQ(t["newmap"]["newseq1"][1].val(), "foo");
        sz = t.lookup_path_or_modify("bar", "newmap.newseq1[2][1]");
        EXPECT_EQ(t["newmap"]["newseq1"].is_seq(), true);
        EXPECT_EQ(t["newmap"]["newseq1"].num_children(), 3u);
        EXPECT_EQ(t["newmap"]["newseq1"][0].val(), nullptr);
        EXPECT_EQ(t["newmap"]["newseq1"][1].val(), "foo");
        EXPECT_EQ(t["newmap"]["newseq1"][2].is_seq(), true);
        EXPECT_EQ(t["newmap"]["newseq1"][2].num_children(), 2u);
        EXPECT_EQ(t["newmap"]["newseq1"][2][0].val(), nullptr);
        EXPECT_EQ(t["newmap"]["newseq1"][2][1].val(), "bar");
        sz = t.lookup_path_or_modify("Foo?"   , "newmap.newseq1[0]");
        sz = t.lookup_path_or_modify("Bar?"   , "newmap.newseq1[2][0]");
        sz = t.lookup_path_or_modify("happy"  , "newmap.newseq1[2][2][3]");
        sz = t.lookup_path_or_modify("trigger", "newmap.newseq1[2][2][2]");
        sz = t.lookup_path_or_modify("Arnold" , "newmap.newseq1[2][2][0]");
        sz = t.lookup_path_or_modify("is"     , "newmap.newseq1[2][2][1]");
        EXPECT_EQ(t["newmap"]["newseq1"].is_seq(), true);
        EXPECT_EQ(t["newmap"]["newseq1"].num_children(), 3u);
        EXPECT_EQ(t["newmap"]["newseq1"][0].val(), "Foo?");
        EXPECT_EQ(t["newmap"]["newseq1"][1].val(), "foo");
        EXPECT_EQ(t["newmap"]["newseq1"][2].is_seq(), true);
        EXPECT_EQ(t["newmap"]["newseq1"][2].num_children(), 3u);
        EXPECT_EQ(t["newmap"]["newseq1"][2][0].val(), "Bar?");
        EXPECT_EQ(t["newmap"]["newseq1"][2][1].val(), "bar");
        EXPECT_EQ(t["newmap"]["newseq1"][2][2].is_seq(), true);
        EXPECT_EQ(t["newmap"]["newseq1"][2][2].num_children(), 4u);
        EXPECT_EQ(t["newmap"]["newseq1"][2][2][0].val(), "Arnold");
        EXPECT_EQ(t["newmap"]["newseq1"][2][2][1].val(), "is");
        EXPECT_EQ(t["newmap"]["newseq1"][2][2][2].val(), "trigger");
        EXPECT_EQ(t["newmap"]["newseq1"][2][2][3].val(), "happy");

        EXPECT_EQ(emitrs<std::string>(t), R"(newmap:
  newseq:
    - newmap:
        newseq:
          - first: y
  newseq1:
    - 'Foo?'
    - foo
    - - 'Bar?'
      - bar
      - - Arnold
        - is
        - trigger
        - happy
newmap2:
  newseq2:
    - ~
    - ~
    - newmap2:
        newseq2:
          - ~
          - ~
          - ~
          - first2: y
            second2: z
)");
    }
}



//-----------------------------------------------------------------------------

TEST(set_root_as_stream, empty_tree)
{
    Tree t;
    NodeRef r = t.rootref();
    EXPECT_EQ(r.is_stream(), false);
    EXPECT_EQ(r.num_children(), 0u);
    t.set_root_as_stream();
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.num_children(), 0u);
}

TEST(set_root_as_stream, already_with_stream)
{
    Tree t;
    t.to_stream(t.root_id());
    NodeRef r = t.rootref();
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.num_children(), 0u);
    t.set_root_as_stream();
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.num_children(), 0u);
}


TEST(set_root_as_stream, root_is_map)
{
    Tree t = parse_in_arena(R"({a: b, c: d})");
    NodeRef r = t.rootref();
    EXPECT_EQ(r.is_stream(), false);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), true);
    EXPECT_EQ(r.is_seq(), false);
    EXPECT_EQ(r.num_children(), 2u);
    print_tree(t);
    std::cout << t;
    t.set_root_as_stream();
    print_tree(t);
    std::cout << t;
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 1u);
    EXPECT_EQ(r[0].is_doc(), true);
    EXPECT_EQ(r[0].is_map(), true);
    EXPECT_EQ(r[0].is_seq(), false);
    EXPECT_EQ(r[0].num_children(), 2u);
    EXPECT_EQ(r[0]["a"].is_keyval(), true);
    EXPECT_EQ(r[0]["c"].is_keyval(), true);
    EXPECT_EQ(r[0]["a"].val(), "b");
    EXPECT_EQ(r[0]["c"].val(), "d");
}

TEST(set_root_as_stream, root_is_docmap)
{
    Tree t = parse_in_arena(R"({a: b, c: d})");
    t._p(t.root_id())->m_type.add(DOC);
    NodeRef r = t.rootref();
    EXPECT_EQ(r.is_stream(), false);
    EXPECT_EQ(r.is_doc(), true);
    EXPECT_EQ(r.is_map(), true);
    EXPECT_EQ(r.is_seq(), false);
    EXPECT_EQ(r.num_children(), 2u);
    print_tree(t);
    std::cout << t;
    t.set_root_as_stream();
    print_tree(t);
    std::cout << t;
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 1u);
    EXPECT_EQ(r[0].is_doc(), true);
    EXPECT_EQ(r[0].is_map(), true);
    EXPECT_EQ(r[0].is_seq(), false);
    EXPECT_EQ(r[0].num_children(), 2u);
    EXPECT_EQ(r[0]["a"].is_keyval(), true);
    EXPECT_EQ(r[0]["c"].is_keyval(), true);
    EXPECT_EQ(r[0]["a"].val(), "b");
    EXPECT_EQ(r[0]["c"].val(), "d");
}


TEST(set_root_as_stream, root_is_seq)
{
    Tree t = parse_in_arena(R"([a, b, c, d])");
    NodeRef r = t.rootref();
    EXPECT_EQ(r.is_stream(), false);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 4u);
    print_tree(t);
    std::cout << t;
    t.set_root_as_stream();
    print_tree(t);
    std::cout << t;
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 1u);
    EXPECT_EQ(r[0].is_doc(), true);
    EXPECT_EQ(r[0].is_map(), false);
    EXPECT_EQ(r[0].is_seq(), true);
    EXPECT_EQ(r[0].num_children(), 4u);
    EXPECT_EQ(r[0][0].val(), "a");
    EXPECT_EQ(r[0][1].val(), "b");
    EXPECT_EQ(r[0][2].val(), "c");
    EXPECT_EQ(r[0][3].val(), "d");
}

TEST(set_root_as_stream, root_is_docseq)
{
    Tree t = parse_in_arena(R"([a, b, c, d])");
    t._p(t.root_id())->m_type.add(DOC);
    NodeRef r = t.rootref();
    EXPECT_EQ(r.is_stream(), false);
    EXPECT_EQ(r.is_doc(), true);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 4u);
    print_tree(t);
    std::cout << t;
    t.set_root_as_stream();
    print_tree(t);
    std::cout << t;
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 1u);
    EXPECT_EQ(r[0].is_doc(), true);
    EXPECT_EQ(r[0].is_map(), false);
    EXPECT_EQ(r[0].is_seq(), true);
    EXPECT_EQ(r[0].num_children(), 4u);
    EXPECT_EQ(r[0][0].val(), "a");
    EXPECT_EQ(r[0][1].val(), "b");
    EXPECT_EQ(r[0][2].val(), "c");
    EXPECT_EQ(r[0][3].val(), "d");
}

TEST(set_root_as_stream, root_is_seqmap)
{
    Tree t = parse_in_arena(R"([{a: b, c: d}, {e: e, f: f}, {g: g, h: h}, {i: i, j: j}])");
    NodeRef r = t.rootref();
    EXPECT_EQ(r.is_stream(), false);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 4u);
    print_tree(t);
    std::cout << t;
    t.set_root_as_stream();
    print_tree(t);
    std::cout << t;
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 1u);
    EXPECT_EQ(r[0].is_doc(), true);
    EXPECT_EQ(r[0].is_map(), false);
    EXPECT_EQ(r[0].is_seq(), true);
    EXPECT_EQ(r[0].num_children(), 4u);
    EXPECT_EQ(r[0][0].is_map(), true);
    EXPECT_EQ(r[0][1].is_map(), true);
    EXPECT_EQ(r[0][2].is_map(), true);
    EXPECT_EQ(r[0][3].is_map(), true);
    EXPECT_EQ(r[0][0]["a"].val(), "b");
    EXPECT_EQ(r[0][0]["c"].val(), "d");
    EXPECT_EQ(r[0][1]["e"].val(), "e");
    EXPECT_EQ(r[0][1]["f"].val(), "f");
    EXPECT_EQ(r[0][2]["g"].val(), "g");
    EXPECT_EQ(r[0][2]["h"].val(), "h");
    EXPECT_EQ(r[0][3]["i"].val(), "i");
    EXPECT_EQ(r[0][3]["j"].val(), "j");
}

TEST(set_root_as_stream, root_is_mapseq)
{
    Tree t = parse_in_arena(R"({a: [0, 1, 2], b: [3, 4, 5], c: [6, 7, 8]})");
    NodeRef r = t.rootref();
    EXPECT_EQ(r.is_stream(), false);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), true);
    EXPECT_EQ(r.is_seq(), false);
    EXPECT_EQ(r.num_children(), 3u);
    print_tree(t);
    std::cout << t;
    t.set_root_as_stream();
    print_tree(t);
    std::cout << t;
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.num_children(), 1u);
    EXPECT_EQ(r[0].is_doc(), true);
    EXPECT_EQ(r[0].is_map(), true);
    EXPECT_EQ(r[0].is_seq(), false);
    EXPECT_EQ(r[0].num_children(), 3u);
    EXPECT_EQ(r[0]["a"].is_seq(), true);
    EXPECT_EQ(r[0]["b"].is_seq(), true);
    EXPECT_EQ(r[0]["c"].is_seq(), true);
    EXPECT_EQ(r[0]["a"][0].val(), "0");
    EXPECT_EQ(r[0]["a"][1].val(), "1");
    EXPECT_EQ(r[0]["a"][2].val(), "2");
    EXPECT_EQ(r[0]["b"][0].val(), "3");
    EXPECT_EQ(r[0]["b"][1].val(), "4");
    EXPECT_EQ(r[0]["b"][2].val(), "5");
    EXPECT_EQ(r[0]["c"][0].val(), "6");
    EXPECT_EQ(r[0]["c"][1].val(), "7");
    EXPECT_EQ(r[0]["c"][2].val(), "8");
}

TEST(set_root_as_stream, root_is_docval)
{
    Tree t;
    NodeRef r = t.rootref();
    r.set_type(DOCVAL);
    r.set_val("bar");
    r.set_val_tag("<!foo>");
    EXPECT_EQ(r.is_stream(), false);
    EXPECT_EQ(r.is_doc(), true);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), false);
    EXPECT_EQ(r.is_val(), true);
    EXPECT_EQ(r.has_val_tag(), true);
    EXPECT_EQ(r.val_tag(), "<!foo>");
    EXPECT_EQ(r.num_children(), 0u);
    print_tree(t);
    std::cout << t;
    t.set_root_as_stream();
    print_tree(t);
    std::cout << t;
    EXPECT_EQ(r.is_stream(), true);
    EXPECT_EQ(r.is_doc(), false);
    EXPECT_EQ(r.is_map(), false);
    EXPECT_EQ(r.is_seq(), true);
    EXPECT_EQ(r.is_val(), false);
    ASSERT_EQ(r.num_children(), 1u);
    EXPECT_EQ(r[0].is_stream(), false);
    EXPECT_EQ(r[0].is_doc(), true);
    EXPECT_EQ(r[0].is_map(), false);
    EXPECT_EQ(r[0].is_seq(), false);
    EXPECT_EQ(r[0].is_val(), true);
    EXPECT_EQ(r[0].has_val_tag(), true);
    EXPECT_EQ(r[0].val_tag(), "<!foo>");
    EXPECT_EQ(r[0].num_children(), 0u);
}



//-------------------------------------------
//-------------------------------------------
//-------------------------------------------

TEST(Tree, doc)
{
    Tree t = parse_in_arena(R"(---
doc0
---
doc1
---
doc2
---
doc3
---
doc4
)");
    size_t ir = t.root_id();
    ASSERT_EQ(t.num_children(ir), 5u);
    ASSERT_TRUE(t.is_stream(ir));
    EXPECT_EQ(t.child(ir, 0), t.doc(0));
    EXPECT_EQ(t.child(ir, 1), t.doc(1));
    EXPECT_EQ(t.child(ir, 2), t.doc(2));
    EXPECT_EQ(t.child(ir, 3), t.doc(3));
    EXPECT_EQ(t.child(ir, 4), t.doc(4));
    {
        NodeRef r = t.rootref();
        EXPECT_EQ(r.id(), ir);
        EXPECT_EQ(r.child(0), r.doc(0));
        EXPECT_EQ(r.child(1), r.doc(1));
        EXPECT_EQ(r.child(2), r.doc(2));
        EXPECT_EQ(r.child(3), r.doc(3));
        EXPECT_EQ(r.child(4), r.doc(4));
        EXPECT_EQ(r.child(0).id(), t.doc(0));
        EXPECT_EQ(r.child(1).id(), t.doc(1));
        EXPECT_EQ(r.child(2).id(), t.doc(2));
        EXPECT_EQ(r.child(3).id(), t.doc(3));
        EXPECT_EQ(r.child(4).id(), t.doc(4));
        EXPECT_EQ(r.child(0).id(), t.docref(0).id());
        EXPECT_EQ(r.child(1).id(), t.docref(1).id());
        EXPECT_EQ(r.child(2).id(), t.docref(2).id());
        EXPECT_EQ(r.child(3).id(), t.docref(3).id());
        EXPECT_EQ(r.child(4).id(), t.docref(4).id());
    }
    {
        const Tree &ct = t;
        const NodeRef r = ct.rootref();
        EXPECT_EQ(r.id(), ir);
        EXPECT_EQ(r.child(0), r.doc(0));
        EXPECT_EQ(r.child(1), r.doc(1));
        EXPECT_EQ(r.child(2), r.doc(2));
        EXPECT_EQ(r.child(3), r.doc(3));
        EXPECT_EQ(r.child(4), r.doc(4));
        EXPECT_EQ(r.child(0).id(), t.doc(0));
        EXPECT_EQ(r.child(1).id(), t.doc(1));
        EXPECT_EQ(r.child(2).id(), t.doc(2));
        EXPECT_EQ(r.child(3).id(), t.doc(3));
        EXPECT_EQ(r.child(4).id(), t.doc(4));
        EXPECT_EQ(r.child(0).id(), t.docref(0).id());
        EXPECT_EQ(r.child(1).id(), t.docref(1).id());
        EXPECT_EQ(r.child(2).id(), t.docref(2).id());
        EXPECT_EQ(r.child(3).id(), t.docref(3).id());
        EXPECT_EQ(r.child(4).id(), t.docref(4).id());
    }
}


//-------------------------------------------
// this is needed to use the test case library
Case const* get_case(csubstr /*name*/)
{
    return nullptr;
}

} // namespace yml
} // namespace c4

#if defined(_MSC_VER)
#   pragma warning(pop)
#elif defined(__clang__)
#   pragma clang diagnostic pop
#elif defined(__GNUC__)
#   pragma GCC diagnostic pop
#endif
