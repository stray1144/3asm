
generator_result_t generator_process(translation_unit_t *output, semantizer_t *source, uint32_t i) {
        generator_result_t result = generator_result(GENERATOR_OK, i);

        switch(semantizer_stream_get(source, i)) {

                case SEMANTIC_COMMENT:
                break;

                case SEMANTIC_NEWLINE:
                break;

                default:
                result = generator_result(GENERATOR_UNEXPECTED, i);
                break;
        }

        return result;
}

generator_result_t generate(translation_unit_t *output, semantizer_t *source) {
        generator_result_t result = generator_result(GENERATOR_OK, 0);

        for(uint32_t i = 0; i < semantizer_stream_size(source); i++) {
                semantizer_unit_kind_t kind = semantizer_stream_get(source, i);
                result = generator_process(output, source, i);
                if(result.status != GENERATOR_OK) break;
        }

        return result;
}










