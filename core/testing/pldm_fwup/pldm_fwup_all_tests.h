#ifndef PLDM_FWUP_ALL_TESTS_H_
#define PLDM_FWUP_ALL_TESTS_H_


#include "testing.h"
#include "platform_all_tests.h"
#include "common/unused.h"


static void add_all_pldm_fwup_tests (CuSuite *suite)
{
    UNUSED (suite);

    TESTING_RUN_SUITE (pldm_fwup_test_complete_fwup);

    TESTING_RUN_SUITE (pldm_fwup_test_idle);
    
    TESTING_RUN_SUITE (pldm_fwup_test_learn_components);

    TESTING_RUN_SUITE (pldm_fwup_test_ready_xfer);

    TESTING_RUN_SUITE (pldm_fwup_test_download);
}





#endif /* PLDM_FWUP_ALL_TESTS_H_ */