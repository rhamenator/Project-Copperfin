# Copyright © 2026 Richard M. Hamilton.
# SPDX-License-Identifier: GPL-3.0-only
# Additional permission: Copperfin Application, Runtime, and Toolchain Exception 1.0; see LICENSE.

# One-request, base-R-only sidecar for the Copperfin v1 polyglot envelope.

MAXIMUM_DOCUMENT_BYTES <- 1024L * 1024L
MAXIMUM_JSON_DEPTH <- 32L
MAXIMUM_JSON_VALUES <- 4096L
MAXIMUM_SAMPLE_VALUES <- 1024L
MAXIMUM_ABSOLUTE_VALUE <- 1e12
CAPABILITY_ID <- "samples.r.mean-v1"
PROTOCOL_VERSION <- "1.0.0"

fail_parse <- function() {
  stop("invalid JSON request", call. = FALSE)
}

new_parser <- function(text) {
  state <- new.env(parent = emptyenv())
  state$text <- text
  state$position <- 1L
  state$length <- nchar(text, type = "chars")
  state$value_count <- 0L
  state
}

peek_character <- function(state) {
  if (state$position > state$length) "" else
    substr(state$text, state$position, state$position)
}

take_character <- function(state) {
  value <- peek_character(state)
  if (identical(value, "")) fail_parse()
  state$position <- state$position + 1L
  value
}

skip_whitespace <- function(state) {
  while (peek_character(state) %in% c(" ", "\t", "\r", "\n")) {
    state$position <- state$position + 1L
  }
}

expect_character <- function(state, expected) {
  if (!identical(take_character(state), expected)) fail_parse()
}

parse_hex_escape <- function(state) {
  if (state$position + 3L > state$length) fail_parse()
  token <- substr(state$text, state$position, state$position + 3L)
  if (!grepl("^[0-9A-Fa-f]{4}$", token, perl = TRUE)) fail_parse()
  state$position <- state$position + 4L
  strtoi(token, base = 16L)
}

parse_json_string <- function(state) {
  expect_character(state, "\"")
  output <- character(state$length - state$position + 1L)
  output_count <- 0L
  append_output <- function(value) {
    output_count <<- output_count + 1L
    output[[output_count]] <<- value
  }
  repeat {
    character <- take_character(state)
    if (identical(character, "\"")) break
    if (identical(character, "\\")) {
      escape <- take_character(state)
      simple <- c(
        "\"" = "\"", "\\" = "\\", "/" = "/", "b" = "\b",
        "f" = "\f", "n" = "\n", "r" = "\r", "t" = "\t")
      if (escape %in% names(simple)) {
        append_output(unname(simple[[escape]]))
      } else if (identical(escape, "u")) {
        codepoint <- parse_hex_escape(state)
        if (codepoint >= 0xD800L && codepoint <= 0xDBFFL) {
          if (!identical(take_character(state), "\\") ||
              !identical(take_character(state), "u")) fail_parse()
          low <- parse_hex_escape(state)
          if (low < 0xDC00L || low > 0xDFFFL) fail_parse()
          codepoint <- 0x10000L +
            (codepoint - 0xD800L) * 0x400L + (low - 0xDC00L)
        } else if (codepoint >= 0xDC00L && codepoint <= 0xDFFFL) {
          fail_parse()
        }
        append_output(intToUtf8(codepoint))
      } else {
        fail_parse()
      }
    } else {
      if (utf8ToInt(character)[[1L]] < 0x20L) fail_parse()
      append_output(character)
    }
  }
  paste0(output[seq_len(output_count)], collapse = "")
}

parse_json_number <- function(state) {
  remaining <- substr(state$text, state$position, state$length)
  match <- regexpr(
    "^-?(0|[1-9][0-9]*)(\\.[0-9]+)?([eE][+-]?[0-9]+)?",
    remaining,
    perl = TRUE)
  if (match[[1L]] != 1L) fail_parse()
  token_length <- attr(match, "match.length")
  token <- substr(remaining, 1L, token_length)
  state$position <- state$position + token_length
  value <- suppressWarnings(as.numeric(token))
  if (length(value) != 1L || !is.finite(value)) fail_parse()
  value
}

parse_json_value <- NULL

parse_json_array <- function(state, depth) {
  expect_character(state, "[")
  result <- list()
  skip_whitespace(state)
  if (identical(peek_character(state), "]")) {
    state$position <- state$position + 1L
    return(structure(result, class = c("copperfin_json_array", "list")))
  }
  repeat {
    result[[length(result) + 1L]] <- parse_json_value(state, depth + 1L)
    skip_whitespace(state)
    separator <- take_character(state)
    if (identical(separator, "]")) break
    if (!identical(separator, ",")) fail_parse()
    skip_whitespace(state)
  }
  structure(result, class = c("copperfin_json_array", "list"))
}

parse_json_object <- function(state, depth) {
  expect_character(state, "{")
  result <- list()
  keys <- new.env(hash = TRUE, parent = emptyenv())
  skip_whitespace(state)
  if (identical(peek_character(state), "}")) {
    state$position <- state$position + 1L
    return(structure(result, class = c("copperfin_json_object", "list")))
  }
  repeat {
    if (!identical(peek_character(state), "\"")) fail_parse()
    key <- parse_json_string(state)
    key_token <- paste0("key:", key)
    if (exists(key_token, envir = keys, inherits = FALSE)) fail_parse()
    assign(key_token, TRUE, envir = keys)
    skip_whitespace(state)
    expect_character(state, ":")
    skip_whitespace(state)
    result[[length(result) + 1L]] <- parse_json_value(state, depth + 1L)
    names(result)[length(result)] <- key
    skip_whitespace(state)
    separator <- take_character(state)
    if (identical(separator, "}")) break
    if (!identical(separator, ",")) fail_parse()
    skip_whitespace(state)
  }
  structure(result, class = c("copperfin_json_object", "list"))
}

parse_json_value <- function(state, depth = 0L) {
  if (depth > MAXIMUM_JSON_DEPTH) fail_parse()
  state$value_count <- state$value_count + 1L
  if (state$value_count > MAXIMUM_JSON_VALUES) fail_parse()
  skip_whitespace(state)
  character <- peek_character(state)
  if (identical(character, "\"")) return(parse_json_string(state))
  if (identical(character, "{")) return(parse_json_object(state, depth))
  if (identical(character, "[")) return(parse_json_array(state, depth))
  if (grepl("[-0-9]", character, perl = TRUE)) {
    return(parse_json_number(state))
  }
  remaining <- substr(state$text, state$position, state$length)
  literals <- list("true" = TRUE, "false" = FALSE,
                   "null" = structure(list(), class = "copperfin_json_null"))
  for (token in names(literals)) {
    if (startsWith(remaining, token)) {
      state$position <- state$position + nchar(token)
      return(literals[[token]])
    }
  }
  fail_parse()
}

parse_document <- function(text) {
  state <- new_parser(text)
  value <- parse_json_value(state)
  skip_whitespace(state)
  if (state$position <= state$length) fail_parse()
  value
}

is_json_object <- function(value) inherits(value, "copperfin_json_object")
is_json_array <- function(value) inherits(value, "copperfin_json_array")

has_exact_keys <- function(value, expected) {
  is_json_object(value) && length(value) == length(expected) &&
    identical(sort(names(value)), sort(expected))
}

escape_json_string <- function(value) {
  codepoints <- utf8ToInt(enc2utf8(value))
  escaped <- vapply(codepoints, function(codepoint) {
    if (codepoint == 0x22L) return("\\\"")
    if (codepoint == 0x5CL) return("\\\\")
    if (codepoint == 0x08L) return("\\b")
    if (codepoint == 0x0CL) return("\\f")
    if (codepoint == 0x0AL) return("\\n")
    if (codepoint == 0x0DL) return("\\r")
    if (codepoint == 0x09L) return("\\t")
    if (codepoint < 0x20L) return(sprintf("\\u%04x", codepoint))
    intToUtf8(codepoint)
  }, character(1L), USE.NAMES = FALSE)
  paste0(escaped, collapse = "")
}

identity_prefix <- function(correlation_id) {
  paste0(
    "{\"envelope_version\":\"1.0\",\"capability_id\":\"",
    CAPABILITY_ID,
    "\",\"correlation_id\":\"", escape_json_string(correlation_id),
    "\",\"protocol_version\":\"", PROTOCOL_VERSION, "\"")
}

emit_document <- function(document) {
  cat(enc2utf8(document), file = stdout(), sep = "")
}

emit_error <- function(correlation_id, code, message) {
  document <- paste0(
    identity_prefix(correlation_id),
    ",\"kind\":\"error\",\"error\":{\"code\":\"", code,
    "\",\"message\":\"", escape_json_string(message),
    "\",\"retryable\":false}}")
  emit_document(document)
}

format_json_number <- function(value) {
  if (identical(value, 0) || value == 0) return("0")
  formatted <- format(
    value, digits = 17L, scientific = FALSE, trim = TRUE,
    decimal.mark = ".")
  if (!grepl("^-?(0|[1-9][0-9]*)(\\.[0-9]+)?$", formatted, perl = TRUE)) {
    fail_parse()
  }
  formatted
}

main <- function() {
  input <- file("stdin", open = "rb")
  on.exit(close(input), add = TRUE)
  raw <- readBin(input, what = "raw", n = MAXIMUM_DOCUMENT_BYTES + 1L)
  if (length(raw) > MAXIMUM_DOCUMENT_BYTES) return(2L)
  text <- tryCatch(rawToChar(raw), error = function(error) NULL)
  if (is.null(text) || !validUTF8(text)) return(2L)
  request <- tryCatch(parse_document(text), error = function(error) NULL)

  required <- c(
    "envelope_version", "kind", "capability_id", "correlation_id",
    "protocol_version", "arguments")
  if (is.null(request) || !has_exact_keys(request, required) ||
      !identical(request$envelope_version, "1.0") ||
      !identical(request$kind, "invocation") ||
      !identical(request$capability_id, CAPABILITY_ID) ||
      !is.character(request$correlation_id) ||
      length(request$correlation_id) != 1L ||
      !nzchar(request$correlation_id) ||
      !identical(request$protocol_version, PROTOCOL_VERSION)) {
    return(2L)
  }

  arguments <- request$arguments
  values <- if (has_exact_keys(arguments, "values")) arguments$values else NULL
  valid_values <- is_json_array(values) && length(values) >= 1L &&
    length(values) <= MAXIMUM_SAMPLE_VALUES &&
    all(vapply(values, function(value) {
      is.numeric(value) && length(value) == 1L && is.finite(value) &&
        abs(value) <= MAXIMUM_ABSOLUTE_VALUE
    }, logical(1L)))
  if (!valid_values) {
    emit_error(
      request$correlation_id,
      "sample.r.invalid_arguments",
      "The sample requires one to 1024 finite numeric values within the documented range.")
    return(0L)
  }

  sum <- 0
  compensation <- 0
  for (value in values) {
    adjusted <- value - compensation
    next_sum <- sum + adjusted
    compensation <- (next_sum - sum) - adjusted
    sum <- next_sum
  }
  mean <- sum / length(values)
  if (!is.finite(mean)) {
    emit_error(
      request$correlation_id,
      "sample.r.invalid_result",
      "The requested mean did not produce a finite result.")
    return(0L)
  }

  document <- paste0(
    identity_prefix(request$correlation_id),
    ",\"kind\":\"success\",\"payload\":{\"mean\":",
    format_json_number(mean), "}}")
  emit_document(document)
  0L
}

quit(status = main(), save = "no", runLast = FALSE)
