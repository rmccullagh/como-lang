   Object *code = newArray(4);
    arrayPushEx(code, newPointer((void *)create_op(LOAD_CONST, newString("Ryan"))));
    arrayPushEx(code, newPointer((void *)create_op(IPRINT, NULL)));
    arrayPushEx(code, newPointer((void *)create_op(LOAD_NAME, newString("name"))));
    arrayPushEx(code, newPointer((void *)create_op(IPRINT, NULL)));
    //arrayPushEx(code, newPointer((void *)create_op(LOAD_NAME, newString("__FUNCTION__"))));
    //arrayPushEx(code, newPointer((void *)create_op(IPRINT, NULL)));
   // arrayPushEx(code, newPointer((void *)create_op(LOAD_NAME, newString("area"))));
   // arrayPushEx(code, newPointer((void *)create_op(CALL_FUNCTION, newString("area"))));
   // arrayPushEx(code, newPointer((void *)create_op(LOAD_CONST, newLong(5L))));
   // arrayPushEx(code, newPointer((void *)create_op(LOAD_CONST, newLong(5L)))); 
   // arrayPushEx(code, newPointer((void *)create_op(ITIMES, NULL)));
    arrayPushEx(code, newPointer((void *)create_op(LOAD_CONST, newLong(5L))));
    arrayPushEx(code, newPointer((void *)create_op(LOAD_CONST, newLong(30L)))); 
    arrayPushEx(code, newPointer((void *)create_op(LOAD_CONST, newLong(2L)))); 
    arrayPushEx(code, newPointer((void *)create_op(LOAD_NAME, newString("do_times"))));
    arrayPushEx(code, newPointer((void *)create_op(CALL_FUNCTION, newString("do_times"))));
    arrayPushEx(code, newPointer((void *)create_op(STORE_NAME, newString("retval"))));
    //arrayPushEx(code, newPointer((void *)create_op(LOAD_NAME, newString("left"))));
    //arrayPushEx(code, newPointer((void *)create_op(IPRINT, NULL)));

    arrayPushEx(code, newPointer((void *)create_op(LOAD_NAME, newString("retval"))));
    arrayPushEx(code, newPointer((void *)create_op(IRETURN, newLong(1L))));

    ComoFrame *frame = create_frame(code);

    Object *code2 = newArray(4);

    arrayPushEx(code2, newPointer((void *)create_op(LOAD_CONST, newString("Como"))));
    arrayPushEx(code2, newPointer((void *)create_op(STORE_NAME, newString("name"))));
    arrayPushEx(code2, newPointer((void *)create_op(LOAD_CONST, newLong(0L)))); 
    arrayPushEx(code2, newPointer((void *)create_op(LOAD_NAME, newString("area"))));
    arrayPushEx(code2, newPointer((void *)create_op(CALL_FUNCTION, newString("area"))));
    arrayPushEx(code2, newPointer((void *)create_op(STORE_NAME, newString("result"))));
    arrayPushEx(code2, newPointer((void *)create_op(LOAD_NAME, newString("result"))));
    arrayPushEx(code2, newPointer((void *)create_op(IPRINT, NULL)));
    arrayPushEx(code2, newPointer((void *)create_op(HALT, NULL)));

    ComoFrame *globalframe = create_frame(code2);

    Object *do_times_code = newArray(4);
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_NAME, newString("__FUNCTION__"))));
    arrayPushEx(do_times_code, newPointer((void *)create_op(IPRINT, NULL)));
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_NAME, newString("left"))));
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_NAME, newString("right"))));
    
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_CONST, newLong(2L)))); 
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_NAME, newString("do_times"))));
    arrayPushEx(do_times_code, newPointer((void *)create_op(CALL_FUNCTION, newString("do_times"))));

    //arrayPushEx(do_times_code, newPointer((void *)create_op(ITIMES, NULL)));
    arrayPushEx(do_times_code, newPointer((void *)create_op(LOAD_CONST, newLong(2L)))); 
    arrayPushEx(do_times_code, newPointer((void *)create_op(IRETURN, newLong(1L))));

    Object *do_times_parameters = newArray(2);
    arrayPushEx(do_times_parameters, newString("left"));
    arrayPushEx(do_times_parameters, newString("right"));

    ComoFrame *do_times_frame = create_frame(do_times_code);
    do_times_frame->namedparameters = do_times_parameters;

    mapInsertEx(globalframe->cf_symtab, "do_times", newPointer((void *)do_times_frame));
    mapInsertEx(globalframe->cf_symtab, "area", newPointer((void *)frame));
    mapInsertEx(globalframe->cf_symtab, "__FUNCTION__", newString("__main__"));

    (void)ex(globalframe);